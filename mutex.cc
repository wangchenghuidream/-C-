#include "mutex.h"
//#include "macro.h"
#include <stdexcept>
namespace sylar
{

Semaphore::Semaphore(uint32_t count) 
{
	//int sem_init(sem_t* _sem, int _pshared, unsigned int _value )
    //_sem为指向信号量的指针，_value为信号量的初始值，_pshared表示是否在进程间共享，不为0则进程间共享，为0则只能当前进程的所有线程共享
    if(sem_init(&m_semaphore, 0, count)) 
	{
		throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore()
{
    //释放信号量占用的一切资源
    sem_destroy(&m_semaphore);
}

void Semaphore::wait()
{
    if(sem_wait(&m_semaphore))
    {
        throw std::logic_error("sem_wait error");
    }
}

void Semaphore::notify()
{
    if(sem_post(&m_semaphore))
    {
        throw std::logic_error("sem_post error");
    }
}
	
	
}
