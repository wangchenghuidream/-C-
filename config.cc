#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace sylar 
{
static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

ConfigVarBase::ptr Config::LookupBase(const std::string& name) 
{
    //RWMutexType::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}	

}