#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include<string>
#include<stdint.h>
#include<list>
#include<vector>
#include<memory>
#include<iostream>
#include<sstream>
#include<fstream>
#include<stdarg.h>
#include<map>
#include "singleton.h"
#include "mutex.h"
#include "util.h"

//使用流式方式将日志级别level的日志写入到logger
#define SYLAR_LOG_LEVEL(logger, level) \
	if(logger->getLevel() <= level)\
		sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger,level,__FILE__,__LINE__,0,sylar::GetThreadId(),sylar::GetFiberId(),time(0)))).getSS()

//使用流式方式将日志级别DEBUG的日志写入到logger
#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::DEBUG)

//使用流式方式将日志级别INFO的日志写入到logger
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::INFO)

//使用流式方式将日志级别WARN的日志写入到logger
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::WARN)

//使用流式方式将日志级别ERROR的日志写入到logger
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::ERROR)

//使用流式方式将日志级别FATAL的日志写入到logger
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::FATAL)

//使用格式化方式将日志级别level的日志写入到logger
#define SYLAR_LOG_FMT_LEVEL(logger,level,fmt,...)\
	if(logger->getLevel() <= level)\
		sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger,level,__FILE__,__LINE__,0,sylar::GetThreadId(),sylar::GetFiberId(),time(0)))).getEvent()->format(fmt,__VA_ARGS__)

//使用格式化方式将日志级别DEBUG的日志写入到logger
#define SYLAR_LOG_FMT_DEBUG(logger,fmt,...)	SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::DEBUG,fmt,__VA_ARGS__)

//使用格式化方式将日志级别INFO的日志写入到logger
#define SYLAR_LOG_FMT_INFO(logger,fmt,...)	SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::INFO,fmt,__VA_ARGS__)

//使用格式化方式将日志级别WARN的日志写入到logger
#define SYLAR_LOG_FMT_WARN(logger,fmt,...)	SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::WARN,fmt,__VA_ARGS__)

//使用格式化方式将日志级别ERROR的日志写入到logger
#define SYLAR_LOG_FMT_ERROR(logger,fmt,...)	SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::ERROR,fmt,__VA_ARGS__)

//使用格式化方式将日志级别FATAL的日志写入到logger
#define SYLAR_LOG_FMT_FATAL(logger,fmt,...)	SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::FATAL,fmt,__VA_ARGS__)

//获得主日志器
#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()

//获取name的日志器
#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)


namespace sylar
{
class Logger;
class LoggerManager;
//日志级别
class LogLevel
{
public:
    enum Level{
		UNKNOW = 0,     //未知级别
        DEBUG =1,        //Debug级别
        INFO =2,        //Info级别
        WARN = 3,       //warn级别
        ERROR = 4,      //error级别
        FATAL = 5       //fatal级别
    };
	static const char* ToString(LogLevel::Level level);
	static LogLevel::Level FromString(const std::string& str);
};

//日志事件
class LogEvent
{
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t m_line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time);
	//返回文件名
	const char* getFile() const {return m_file;}

	//返回行号
	int32_t getLine() const {return m_line;}

	//返回耗时
	uint32_t getElapse() const {return m_elapse;}

	//返回线程id
	uint32_t getThreadId() const {return m_threadId;}

	//返回协程id
	uint32_t getFiberId() const {return m_fiberId;};

	//返回时间
	uint64_t getTime() const {return m_time;}
	
	//返回日志内容
	const std::string getContent() const {return m_ss.str();}
	
	//返回日志内容字符串
	std::stringstream& getSS()	{return m_ss;}
	
	//返回日志器
	std::shared_ptr<Logger> getLogger() const {return m_logger;}
	
	//返回日志级别
	LogLevel::Level getLevel() const {return m_level;}
	
	//返回线程名称
	const std::string& getThreadName() const { return m_threadName;}
	
	//格式化写入日志内容
	void format(const char* fmt, ...);

	void format(const char* fmt, va_list al);
	
private:
    const char* m_file = nullptr;   //文件名
    int32_t m_line =0;               //行号
    uint32_t m_elapse = 0;           //程序启动到现在的毫秒数
    uint32_t m_threadId = 0;         //线程id
    uint32_t m_fiberId = 0;           //协程Id
    uint64_t m_time = 0;                 //时间戳
	std::string m_threadName;		//线程名称
    std::stringstream m_ss;		//日志内容流
	std::shared_ptr<Logger> m_logger;	//日志器
	LogLevel::Level m_level;		//日志等级
};

//日志事件包装器
class LogEventWrap
{
public:
	LogEventWrap(LogEvent::ptr e);
	~LogEventWrap();

	//获取日志事件
	LogEvent::ptr getEvent() const {return m_event;}

	//获取日志内容流
	std::stringstream& getSS();
private:
	LogEvent::ptr m_event;
};


//日志格式器
class LogFormatter
{
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);
	  //返回格式化日志文本
    std::string format(std::shared_ptr<Logger>, LogLevel::Level level, LogEvent::ptr event);
	std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
	
public:
	//具体的日志格式项
    class FormatItem
    {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
		FormatItem(const std::string& fmt = ""){};
        virtual ~FormatItem(){}
		//将对应的日志格式内容写入到os
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr evect) =0;
    };

	//初始化，解析日志模板
    void init();
	
	bool isError() const {return m_error;}
	const std::string getPattern() const{return m_pattern;}

private:
	//日志格式模板
    std::string m_pattern;
	//通过日志格式解析出来的FormatItem,支持扩展
    std::vector<FormatItem::ptr> m_items;
	//是否出错
	bool m_error = false;

};

//日志输出地
class LogAppender
{
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
	
	typedef SpinLock MutexType;
    virtual ~LogAppender(){}

	  //写入日志
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) =0;
	//更改日志格式器
	void setFormatter(LogFormatter::ptr val);

	//获取日志格式器
	LogFormatter::ptr getFormatter() const {return m_formatter;};
    
	//获取日志级别
	LogLevel::Level getLevel() const {return m_level;}
	
	//设置日志级别
	void setLevel(LogLevel::Level val) { m_level = val;}
	
	virtual std::string toYamlString() =0;
	
protected:
    LogLevel::Level m_level;
	LogFormatter::ptr m_formatter;
	bool m_hasFormatter = false;
	MutexType m_mutex;

};


//日志输出器
//只有继承了std::enable_shared_from_this<Logger>,才能在该类的成员函数内获得该对象的智能指针
class Logger :public std::enable_shared_from_this<Logger>
{
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
	typedef SpinLock MutexType;
	
    Logger(const std::string& name = "root");

    void log(LogLevel::Level level, const LogEvent::ptr event);
    
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);
    
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
	  void clearAppender();

    LogLevel::Level getLevel() const {return m_level;};
    void setLevel(LogLevel::Level val){m_level = val;};
    const std::string& getName() const {return m_name;}
	
	void setFormatter(LogFormatter::ptr val);
	void setFormatter(const std::string& val);
	
	LogFormatter::ptr getFormatter()
	{
		return m_formatter;
	}
	std::string toYamlString();
	
private:
    std::string m_name;     //日志名称
    LogLevel::Level m_level;    //日志级别
    std::list<LogAppender::ptr> m_appenders; //Appender集合
	LogFormatter::ptr m_formatter;
	Logger::ptr m_root;		//主日志器
	MutexType m_mutex;
};

//输出到控制台的Appender
class StdoutLogAppender: public LogAppender
{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
	std::string toYamlString() override;
};

//输出到文件的Appender
class FileLogAppender: public LogAppender
{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
	
    FileLogAppender(const std::string& filename);
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
	
	//重新打开文件
	bool reopen();
	std::string toYamlString() override;
	
private:
	//文件路径
    std::string m_filename;
	//文件流
    std::ofstream m_filestream;

};

//日志器管理类
class LoggerManager
{
public:
	LoggerManager();
	typedef SpinLock MutexType;
	//获取日志器
	Logger::ptr getLogger(const std::string& name);

	//初始化
	void init();

	//返回主日志器
	Logger::ptr getRoot() const {return m_root;}

	//将所有的日志器配置转成YAML String
	//std::string toYamlString();
private:
	std::map<std::string, Logger::ptr> m_loggers;

	//主日志器
	Logger::ptr m_root;
	MutexType m_mutex;
};

typedef sylar::Singleton<LoggerManager> LoggerMgr;


}



#endif // LOG_H_INCLUDED