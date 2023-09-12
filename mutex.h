#ifndef __SYLAR_MUTEX_H__
#define __SYLAR_MUTEX_H__
#include<pthread.h>
#include<semaphore.h>
#include<stdint.h>
#include<atomic>
#include<list>
#include <exception>

#include "noncopyable.h"
//#include "fiber.h"

namespace sylar
{

//信号量
class Semaphore:Noncopyable
{
public:
    //构造函数
    Semaphore(uint32_t count =0);

    //析构函数
    ~Semaphore();

    //获取信号量
    void wait();

    //释放信号量
    void notify();
private:
    //定义信号量，类型为sem_t,实质是一个长整型的数
    sem_t m_semaphore;
};


//局部锁的模板实现
template<class T>
struct ScopedLockImpl
{
public:
    ScopedLockImpl(T &mutex)
    :m_mutex(mutex)
    {
        m_mutex.lock();
        m_locked = true;
    }

    ~ScopedLockImpl()
    {
        unlock();
    }

    void lock()
    {
        if(!m_locked)
        {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock()
    {
        if(m_locked)
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

//局部读锁模板实现
template<class T>
struct ReadScopedLockImpl
{
public:
    ReadScopedLockImpl(T &mutex)
    :m_mutex(mutex)
    {
        m_mutex.rdlock();
        m_locked = true;
    }

    ~ReadScopedLockImpl()
    {
        unlock();
    }

    void lock()
    {
        if(!m_locked)
        {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock()
    {
        if(m_locked)
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

//局部写锁模板实现
template<class T>
struct WriteScopedLockImpl
{
public:
    WriteScopedLockImpl(T &mutex)
    :m_mutex(mutex)
    {
        m_mutex.wrlock();
        m_locked = true;
    }

    ~WriteScopedLockImpl()
    {
        unlock();
    }

    void lock()
    {
        if(!m_locked)
        {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock()
    {
        if(m_locked)
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

class Mutex:Noncopyable
{
public:
    //局部锁
    typedef ScopedLockImpl<Mutex> Lock;

    //构造函数
    Mutex()
    {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    //析构函数
    ~Mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    //加锁
    void lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};

class NullMutex: Noncopyable
{
public:
    //局部锁
    typedef ScopedLockImpl<NullMutex> Lock;

    //构造函数
    NullMutex(){}


    //析构函数
    ~NullMutex(){}

    //加锁
    void lock(){}

    void unlock(){}
};

class RWMutex: Noncopyable
{
 public:
     //局部读锁
     typedef ReadScopedLockImpl<RWMutex> ReadLock;
     //局部写锁
     typedef WriteScopedLockImpl<RWMutex> WriteLock;
     //构造函数
     RWMutex()
     {
         pthread_rwlock_init(&m_lock, nullptr);
     }
     //析构函数
     ~RWMutex()
     {
         pthread_rwlock_destroy(&m_lock);
     }
     //上读锁
     void rdlock()
     {
         pthread_rwlock_rdlock(&m_lock);
     }
     //上写锁
     void wrlock()
     {
         pthread_rwlock_wrlock(&m_lock);
     }
     //解锁
     void unlock()
     {
         pthread_rwlock_unlock(&m_lock);
     }

 private:
    pthread_rwlock_t m_lock;


};

//自旋锁
class SpinLock:Noncopyable
{
public:
    typedef ScopedLockImpl<SpinLock> Lock;

    SpinLock()
    {
        pthread_spin_init(&m_mutex, 0);
    }
    ~SpinLock()
    {
		pthread_spin_destroy(&m_mutex);
    }
    void lock()
    {
        pthread_spin_lock(&m_mutex);
    }
    void unlock()
    {
        pthread_spin_unlock(&m_mutex);
    }

private:
    pthread_spinlock_t m_mutex;
};

//原子锁
class CASLock:Noncopyable
{
public:
    typedef ScopedLockImpl<CASLock> Lock;

    CASLock()
    {
        m_mutex.clear();
    }
    ~CASLock()
    {
    }
    void lock()
    {
        while(std::atomic_flag_test_and_set_explicit(&m_mutex,std::memory_order_acquire));
    }
    void unlock()
    {
        std::atomic_flag_clear_explicit(&m_mutex,std::memory_order_release);
    }

private:
    volatile std::atomic_flag m_mutex;
};


}

#endif // __SYLAR_MUTEX_H__