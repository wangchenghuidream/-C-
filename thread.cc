#include "thread.h"
#include "log.h"
#include "util.h"
namespace sylar
{
//c++中四种存储周期
//automatic
//static
//dynamic：通过动态内存分配函数来分配和销毁的
//thread:变量在线程开始的时候被生成，在线程结束的时候被销毁，用thread_local修饰
//记录当前线程
static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";
static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

Thread* Thread::GetThis()
{
    return t_thread;
}

const std::string& Thread::GetName()
{
    return t_thread_name;
}

void Thread::SetName(const std::string& name)
{
    if(t_thread)
    {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

Thread::Thread(std::function<void()> cb, const std::string& name)
:m_cb(cb), m_name(name)
{
    if(name.empty())
    {
        m_name = "UNKNOW";
    }
    //int pthread_create(pthread_t *thread,
     //              const pthread_attr_t *attr,    用于手动设置新线程的属性
       //            void *(*start_routine) (void *),   以函数指针的方式指明新建线程需要执行的函数
         //          void *arg);    指定传递给start_routine函数的实参
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if(rt)
    {
        SYLAR_LOG_ERROR(g_logger)<<"pthread_create thread fail,rt="<<rt
        <<" name="<<name;
        throw std::logic_error("pthread_create error");
    }
}

Thread::~Thread()
{
    if(m_thread)
    {
        //从状态上实现线程分离
        //线程分离状态：指定该状态，线程主动与主控线程断开关系。线程结束后，其退出状态不由其他线程获取，而直接自己释放，
        pthread_detach(m_thread);
    }
}

void Thread::join()
{
    if(m_thread)
    {
        //会阻塞调用它的线程，直至目标线程执行结束
        int rt = pthread_join(m_thread, nullptr);
        if(rt)
        {
            SYLAR_LOG_ERROR(g_logger)<<"pthread_join thread fail, rt="<<rt
            <<" name="<<m_name;
            throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}

void* Thread::run(void* arg)
{
    Thread* thread = (Thread*)arg;
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = sylar::GetThreadId();

    //pthread_setname_np(pthread_t thread, const char *name)    设置线程名称
    //pthread_getname_np(pthread_t thread, char *name, size_t len)  获取线程名称
    //pthread_self()获得线程id,但仅保证在同一进程中唯一
    pthread_setname_np(pthread_self(), thread->m_name.substr(0,15).c_str());

    std::function<void()> cb;
    cb.swap(thread->m_cb);

    //thread->m_semaphore.notify();
    cb();
    return 0;

}

}
