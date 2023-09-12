#include "fd_manager.h"
#include "hook.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace sylar 
{

FdCtx::FdCtx(int fd):m_isInit(false),m_isSocket(false),m_sysNonblock(false),m_userNonblock(false)
	,m_isClosed(false),m_fd(fd),m_recvTimeout(-1),m_sendTimeout(-1) 
{
    init();
}

FdCtx::~FdCtx() 
{
}

bool FdCtx::init() 
{
    if(m_isInit) 
	{
        return true;
    }
    m_recvTimeout = -1;
    m_sendTimeout = -1;

    /*
    int fstat(int fd, struct stat &buf);
    将fd指向的文件状态复制到buf指向的结构中，fd要求为打开的文件描述符
    成功返回0，失败返回-1
    struct stat{
        dev_t st_dev;//文件所在设备id
        ino_t st_ino;//节点（inode）编号
        mode_t st_mode;//保护模式
        nlink_t st_nlink;//硬链接个数
        uid_t st_uid;//所有者用户ID
        gid_t st_gid;//所有者组ID
        dev_t st_rdev;//设备ID（如果是特殊文件）
        off_t st_size;//总体尺寸，以字节为单位
        blksize_t st_blksize;//文件系统IO块大小
        blkcnt_t st_blocks;//已分配的512B快个数
        time_t st_atime;//上次访问时间
        time_t st_mtime;//上次更新时间
        time_t st_ctime;//上次状态更新时间
    };
    */
    struct stat fd_stat;
    if(-1 == fstat(m_fd, &fd_stat))
    {
        m_isInit = false;
        m_isSocket = false;
    }
    else
    {
        m_isInit = true;
		//
        m_isSocket = S_ISSOCK(fd_stat.st_mode);
    }

    if(m_isSocket)
    {
        //获取fd状态
        int flags = fcntl_f(m_fd, F_GETFL, 0);
		//将这个fd设置为非阻塞
        if(!(flags & O_NONBLOCK))
        {
            fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
        }
        m_sysNonblock = true;
    }
    else
    {
        m_sysNonblock = false;
    }
    m_userNonblock = false;
    m_isClosed = false;
    return m_isInit;
}

void FdCtx::setTimeout(int type, uint64_t v)
{
    if(type == SO_RCVTIMEO)
    {
        m_recvTimeout = v;
    }
    else
    {
        m_sendTimeout = v;
    }
}

uint64_t FdCtx::getTimeout(int type)
{
    if(type == SO_RCVTIMEO)
    {
        return m_recvTimeout;
    }
    else
    {
        return m_sendTimeout;
    }
}

FdManager::FdManager()
{
    m_datas.resize(64);
}

FdCtx::ptr FdManager::get(int fd, bool auto_create)
{
    if(fd == -1)
    {
        return nullptr;
    }
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_datas.size() <= fd)
    {
        if(auto_create == false)
        {
            return nullptr;
        }
    }
    else
    {
        if(m_datas[fd] || !auto_create)
        {
            return m_datas[fd];
        }
    }
    lock.unlock();

    RWMutexType::WriteLock lock2(m_mutex);
    FdCtx::ptr ctx(new FdCtx(fd));
    if(fd >= (int)m_datas.size())
    {
        m_datas.resize(fd * 1.5);
    }
    m_datas[fd] = ctx;
    return ctx;
}

void FdManager::del(int fd)
{
    RWMutexType::WriteLock lock(m_mutex);
    if((int)m_datas.size() <= fd)
    {
        return;
    }
    m_datas[fd].reset();
}

}
