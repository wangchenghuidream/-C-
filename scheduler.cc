#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "util.h"
#include "hook.h"

namespace sylar 
{

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

//定义一个当前协程调度器和当前协程
static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name):m_name(name) 
{
    SYLAR_ASSERT(threads > 0);

    if(use_caller) 
	{
		//记录下当前线程的主协程上下文
        sylar::Fiber::GetThis();
        --threads;

        SYLAR_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        sylar::Thread::SetName(m_name);

        t_scheduler_fiber = m_rootFiber.get();
        m_rootThread = sylar::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    } 
	else 
	{
        m_rootThread = -1;
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler() 
{
    SYLAR_ASSERT(m_stopping);
    if(GetThis() == this) 
	{
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() 
{
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() 
{
    return t_scheduler_fiber;
}

void Scheduler::start() 
{
    MutexType::Lock lock(m_mutex);
	//正在stop
    if(!m_stopping) 
	{
        return;
    }
    m_stopping = false;
    SYLAR_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for(size_t i = 0; i < m_threadCount; ++i) 
	{
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();
}

void Scheduler::stop() 
{
    m_autoStop = true;
	//调度完成后结束，调度协程状态为完成或初始化，无运行线程
    if(m_rootFiber && m_threadCount == 0 && (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::INIT)) 
	{
        SYLAR_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;
		
		//还有协程存在时，继续运行
        if(stopping()) 
		{
            return;
        }
    }
	
    //bool exit_on_this_fiber = false;
	//
    if(m_rootThread != -1) 
	{
        SYLAR_ASSERT(GetThis() == this);
    } 
	else 
	{
        SYLAR_ASSERT(GetThis() != this);
    }
	//为什么再stop()中设置，m_stopping,用于将主线程用于调度，当调用stop()后，
	//意味着用户已经将需要调度的协程放入调度器中了，这时调度器只需要等待所有协程完成调度，所以这时调度器所在线程也可用于调度
    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) 
	{
        tickle();//这里应该实现通知各个线程，调度结束
    }

    if(m_rootFiber) 
	{
        tickle();
    }

    if(m_rootFiber) 
	{
		//其他线程都停止了，但还没完全停止，还有协程运行，则表示需要在主线程上运行
        if(!stopping()) 
		{
            m_rootFiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) 
	{
        i->join();
    }
}

void Scheduler::setThis() 
{
    t_scheduler = this;
}

void Scheduler::run() 
{
    SYLAR_LOG_DEBUG(g_logger) << m_name << " run";
    set_hook_enable(true);
    setThis();
    if(sylar::GetThreadId() != m_rootThread) 
	{
        t_scheduler_fiber = Fiber::GetThis().get();
    }

    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    while(true) 
	{
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()) 
			{
				//不是本线程应该执行的协程则跳过
                if(it->thread != -1 && it->thread != sylar::GetThreadId()) 
				{
                    ++it;
                    tickle_me = true;//可以记录一下有其他线程需要执行的协程
                    continue;
                }

                SYLAR_ASSERT(it->fiber || it->cb);
				//找到协程正在执行中，则跳过
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) 
				{
                    ++it;
                    continue;
                }

                ft = *it;
                m_fibers.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            tickle_me |= it != m_fibers.end();
        }

        if(tickle_me) 
		{
            tickle();
        }
		//找到的协程，处于INIT初始化,HOLD暂停,READY可执行状态
        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT)) 
		{
            ft.fiber->swapIn();
            --m_activeThreadCount;
            if(ft.fiber->getState() == Fiber::READY) 
			{
                schedule(ft.fiber);
            }
			else if(ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT) //？这部分应该可以不要
			{
                ft.fiber->m_state = Fiber::HOLD;
            }
            ft.reset();
        }
		else if(ft.cb) 
		{
            if(cb_fiber) 
			{
                cb_fiber->reset(ft.cb);
            } 
			else 
			{
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) 
			{
                schedule(cb_fiber);
                cb_fiber.reset();
            } 
			else if(cb_fiber->getState() == Fiber::EXCEPT || cb_fiber->getState() == Fiber::TERM) 
			{
                cb_fiber->reset(nullptr);
            } 
			else 
			{//if(cb_fiber->getState() != Fiber::TERM) {
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        } 
		else
		{
            if(is_active)
			{
                --m_activeThreadCount;
                continue;
            }
            if(idle_fiber->getState() == Fiber::TERM) 
			{
                SYLAR_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }

            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT) 
			{
                idle_fiber->m_state = Fiber::HOLD;
            }
        }
    }
}

void Scheduler::tickle() 
{
    SYLAR_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() 
{
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() 
{
    SYLAR_LOG_INFO(g_logger) << "idle";
    while(!stopping()) 
	{
        sylar::Fiber::YieldToHold();
    }
}

void Scheduler::switchTo(int thread) 
{
    SYLAR_ASSERT(Scheduler::GetThis() != nullptr);
    if(Scheduler::GetThis() == this) 
	{
        if(thread == -1 || thread == sylar::GetThreadId()) 
		{
            return;
        }
    }
    schedule(Fiber::GetThis(), thread);
    Fiber::YieldToHold();
}

std::ostream& Scheduler::dump(std::ostream& os) 
{
    os << "[Scheduler name=" << m_name
       << " size=" << m_threadCount
       << " active_count=" << m_activeThreadCount
       << " idle_count=" << m_idleThreadCount
       << " stopping=" << m_stopping
       << " ]" << std::endl << "    ";
    for(size_t i = 0; i < m_threadIds.size(); ++i) 
	{
        if(i) 
		{
            os << ", ";
        }
        os << m_threadIds[i];
    }
    return os;
}


SchedulerSwitcher::SchedulerSwitcher(Scheduler* target) {
    m_caller = Scheduler::GetThis();
    if(target) {
        target->switchTo();
    }
}

SchedulerSwitcher::~SchedulerSwitcher() {
    if(m_caller) {
        m_caller->switchTo();
    }
}

}