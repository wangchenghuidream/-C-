#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__

#include<memory>
#include<functional>
#include<ucontext.h>
#include "thread.h"

namespace sylar
{

class Fiber: public std::enable_shared_from_this<Fiber>
{
friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;
    //协程状态
    enum State
    {
        INIT,   //初始化状态
        HOLD,   //暂停状态
        EXEC,   //执行状态
        TERM,   //结束状态
        READY,  //可执行状态
        EXCEPT  //异常状态
    };

private:
    //无参构造函数，每个线程第一个协程的构造
    Fiber();

public:

    //cd 协程执行的函数
    //stacksize    协程栈大小
    //use_caller    是否在mainfiber上调度
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);

    ~Fiber();

    //重置协程函数，并重置状态
    //INIT,TERM,EXXCEPT
    void reset(std::function<void()> cb);

    //切换到当前协程执行
    void swapIn();

    //把当前协程切换到后台
    void swapOut();

    //将当前线程切换到执行状态，执行的是当前线程的主协程
    void call();

    //将当前线程切换到后台，执行的为该协程，返回到线程的主协程
    void back();

    //返回协程id
    uint64_t getId() const {return m_id;}

    State getState() const {return m_state;}

public:
    //设置当前线程的运行协程
    static void SetThis(Fiber* f);

    //返回当前协程
    static Fiber::ptr GetThis();

    //协程切换到后台，并且设置为Ready状态
    static void YieldToReady();

    //协程切换到后台，并设置为Hold状态
    static void YieldToHold();

    //总协程数
    static uint64_t TotalFibers();

    //协程执行函数，执行完成后回到线程主协程
    static void MainFunc();

    //协程执行函数，执行完成后回到线程调度协程
    static void CallerMainFunc();

    //获得当前协程的id
    static uint64_t GetFiberId();

private:

    //协程id
    uint64_t m_id = 0;
    //协程运行栈大小
    uint32_t m_stacksize = 0;
    //协程状态
    State m_state = INIT;
    //协程上下文
    /*
    typedf struct ucontext_t
    {
        unsigned long int __ctx(uc_flags);
        struct ucontext_t *uc_link; 当前context执行结束之后要执行的下一个context,如果uc_linkl为空，执行完当前context后退出程序
        stack_t uc_stack;   当前上下文所需要的stack
        mcontext_t uc_mcontext; 保存具体的程序执行上下文，如PC值，堆栈指针和寄存器的值，它的实现依赖于底层，是硬件平台相关的，因此不透明
        sigset_t uc_sigmask;    执行上下文过程中，要屏蔽的信号集合，即信号掩码
        struck _libc_fpstate __fprefs_men;
    }
    getcontext(),获得当前上下文
    setcontext(),设置当前上下文
    makecontext(),设置该上下文的执行函数
    swapcontext(),切换上下文

    */
    ucontext_t m_ctx;
    //协程运行栈指针
    void* m_stack = nullptr;
    //协程运行函数
    std::function<void()> m_cb;



};

}

#endif //  __SYLAR_FIBER_H__
