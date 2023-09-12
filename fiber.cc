#include "fiber.h"
//#include "config.h"
#include "macro.h"
#include "log.h"
#include "scheduler.h"
#include<atomic>

namespace sylar
{

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

//将协程号和协程数量定义为原子类型，使得操作具有原子性
static std::atomic<uint64_t> s_fiber_id {0}; //记录已分配的协程号
static std::atomic<uint64_t> s_fiber_count {0}; //记录协程数量

static thread_local Fiber* t_fiber = nullptr;//记录当前的运行协程
static thread_local Fiber::ptr t_threadFiber = nullptr;//记录该线程的主协程(进行协程调度的那个协程)

uint32_t g_fiber_stack_size = 128 * 1024;//不能设置太小，太小了切换协程的时候容易出问题
//static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
  //  Config::Lookup<uint32_t>("fiber.stack_size",1024*1024 , "fiber stack size");


//分配器
class MallocStackAllocator
{
public:
    static void* Alloc(size_t size)
    {
        return malloc(size);
    }
    static void Dealloc(void* vp, size_t size)
    {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId()
{
    if(t_fiber)
    {
        return t_fiber->getId();
    }
    return 0;
}

//每个线程第一个协程的构造
Fiber::Fiber()
{
    m_state = EXEC;
    //将本协程设置为当前协程
    SetThis(this);
    //getcontext(ucontext_t *ucp)   初始化ucp结构体，将当前上下文保存在ucp中，成功返回0，错误返回-1
    if(getcontext(&m_ctx))
    {
        SYLAR_ASSERT2(false, "getcontext");
    }
    ++s_fiber_count;
    SYLAR_LOG_DEBUG(g_logger)<<"Fiber::Fiber main";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller):m_id(++s_fiber_id),m_cb(cb)
{
    ++s_fiber_count;
    m_stacksize = stacksize?stacksize:g_fiber_stack_size;

    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx))
    {
        SYLAR_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link =nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;


    //makecontext(ucontext_t *ucp, void (*func)(), int args, ...)
    //makecontext修改通过getcontext取得的上下文ucp,然后给该上下文指定一个栈空间，设置后继的上下文
    //当上下文通过setcontext或者swapcontext激活后，执行func函数，args为func的参数个数，后面是func的参数序列，当func执行返回后，后继的上下文被激活，如果后继为空，则线程退出
    if(!use_caller)
    {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    }
    else
    {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }
    SYLAR_LOG_DEBUG(g_logger)<<"Fiber::Fiber id = "<<m_id;
}

Fiber::~Fiber()
{
    --s_fiber_count;
    if(m_stack)
    {
        SYLAR_ASSERT(m_state==TERM || m_state==EXCEPT || m_state==INIT);
        StackAllocator::Dealloc(m_stack, m_stacksize);
    }
    else
    {
        SYLAR_ASSERT(!m_cb);
        SYLAR_ASSERT(m_state == EXEC);
        Fiber* cur = t_fiber;
        if(cur == this)
        {
            SetThis(nullptr);
        }
    }
    SYLAR_LOG_DEBUG(g_logger)<<"Fiber::~Fiber id="<<m_id<<" total="<<s_fiber_count;
}

//重置协程函数，并重置状态
void Fiber::reset(std::function<void()> cb)
{
    SYLAR_ASSERT(m_stack);
    //只有协程处于初始化状态，结束状态，异常状态，才可重置其运行函数
    SYLAR_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);
    m_cb = cb;
    if(getcontext(&m_ctx))
    {
        SYLAR_ASSERT2(false,"getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    //getcontext()获得的上下文保存的栈和入口执行点都是原线程的，makecontext()将会为执行上下文指定新的入口点（即指定这个上下文的执行函数是什么）
    /*
    void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...)
    修改ucp指向的上下文，用于指定上下文执行时的入口点函数
    */

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

void Fiber::call()
{
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&(t_threadFiber->m_ctx), &m_ctx))
    {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}


void Fiber::back()
{
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx))
    {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

//切换到当前协程执行
void Fiber::swapIn()
{
    SetThis(this);
    SYLAR_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx))
    {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

//切换到后台执行
void Fiber::swapOut()
{
    SetThis(Scheduler::GetMainFiber());
    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx))
    {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

//设置当前协程
void Fiber::SetThis(Fiber* f)
{
    t_fiber = f;
}

//返回当前协程
Fiber::ptr Fiber::GetThis()
{
    if(t_fiber)
    {
        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fiber(new Fiber);
    SYLAR_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

//协程切换到后台，并设置为Ready状态
void Fiber::YieldToReady()
{
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur->m_state == EXEC);
    cur->m_state = READY;
    cur->swapOut();
}

//协程切换到后台，并且设置为HOLD状态
void Fiber::YieldToHold()
{
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur->m_state == EXEC);
    cur->m_state = HOLD;
    cur->swapOut();
}

//总协程数
uint64_t Fiber::TotalFibers()
{
    return s_fiber_count;
}

void Fiber::MainFunc()
{
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur);
    try
    {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    }catch(std::exception& ex)
    {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except:" <<ex.what()
        << "fiber_id="<<cur->getId()
        <<std::endl
        <<sylar::BacktraceToString();
    }catch(...)
    {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) <<"Fiber Except"
        <<"fiber_id="<<cur->getId()
        <<std::endl
        <<sylar::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
	//raw_ptr->back();
    raw_ptr->swapOut();
    SYLAR_ASSERT2(false,"never reach fiber_id="+std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc()
{
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur);
    try
    {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    }catch(std::exception& ex)
    {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) <<"Fiber Except:"<< ex.what()
        <<"fiber_id="<<cur->getId()
        <<std::endl
        <<sylar::BacktraceToString();
    }catch(...)
    {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except"
        <<"fiber_id="<<cur->getId()
        <<std::endl
        <<sylar::BacktraceToString();
    }
	SYLAR_LOG_ERROR(g_logger) <<"out of";
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    SYLAR_ASSERT2(false, "never reach fiber_id =" + std::to_string(raw_ptr->getId()));

}


}
