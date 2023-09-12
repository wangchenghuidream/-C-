//HTTP定义结构体封装
#ifndef __SYLAR_HTTP_HTTP_H__
#define __SYLAR_HTTP_HTTP_H__

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
//#include <boost/lexical_cast.hpp>

namespace sylar 
{
namespace http 
{

/* Request Methods */
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */  \
  XX(31, LINK,        LINK)         \
  XX(32, UNLINK,      UNLINK)       \
  /* icecast */                     \
  XX(33, SOURCE,      SOURCE)       \

/* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \

//HTTP方法枚举
enum class HttpMethod 
{
#define XX(num, name, string) name = num,
    HTTP_METHOD_MAP(XX)
#undef XX
    INVALID_METHOD
};

// HTTP状态枚举
enum class HttpStatus 
{
#define XX(code, name, desc) name = code,
    HTTP_STATUS_MAP(XX)
#undef XX
};

// 将字符串方法名转成HTTP方法枚举
HttpMethod StringToHttpMethod(const std::string& m);

//将字符串指针转换成HTTP方法枚举
HttpMethod CharsToHttpMethod(const char* m);

// 将HTTP方法枚举转换成字符串
const char* HttpMethodToString(const HttpMethod& m);

// 将HTTP状态枚举转换成字符串
const char* HttpStatusToString(const HttpStatus& s);

// 忽略大小写比较仿函数
struct CaseInsensitiveLess 
{
    bool operator()(const std::string& lhs, const std::string& rhs) const;
};

//获取Map中的key值,并转成对应类型,返回是否成功
template<class MapType, class T>
bool checkGetAs(const MapType& m, const std::string& key, T& val, const T& def = T()) 
{
    auto it = m.find(key);
    if(it == m.end()) 
	{
        val = def;
        return false;
    }
    try 
	{
        //val = boost::lexical_cast<T>(it->second);
		val = static_cast<T>(std::stold(it->second));
        return true;
    } 
	catch (...) 
	{
        val = def;
    }
    return false;
}

// 获取Map中的key值,并转成对应类型
template<class MapType, class T>
T getAs(const MapType& m, const std::string& key, const T& def = T()) 
{
    auto it = m.find(key);
    if(it == m.end()) 
	{
        return def;
    }
    try 
	{
        //return boost::lexical_cast<T>(it->second);
		return static_cast<T>(std::stold(it->second));
    } 
	catch (...) {
    }
    return def;
}

class HttpResponse;

// HTTP请求结构
class HttpRequest {
public:
    // HTTP请求的智能指针
    typedef std::shared_ptr<HttpRequest> ptr;
    // MAP结构
    typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;
    HttpRequest(uint8_t version = 0x11, bool close = true);

    std::shared_ptr<HttpResponse> createResponse();

    //返回HTTP方法
    HttpMethod getMethod() const { return m_method;}

    // 返回HTTP版本
    uint8_t getVersion() const { return m_version;}

    // 返回HTTP请求的路径
    const std::string& getPath() const { return m_path;}

    // 返回HTTP请求的查询参数
    const std::string& getQuery() const { return m_query;}

    // 返回HTTP请求的消息体
    const std::string& getBody() const { return m_body;}

    // 返回HTTP请求的消息头MAP
    const MapType& getHeaders() const { return m_headers;}

    // 返回HTTP请求的参数MAP
    const MapType& getParams() const { return m_params;}

    //返回HTTP请求的cookie MAP
    const MapType& getCookies() const { return m_cookies;}

    // 设置HTTP请求的方法名
    void setMethod(HttpMethod v) { m_method = v;}

    // 设置HTTP请求的协议版本
    void setVersion(uint8_t v) { m_version = v;}

    // 设置HTTP请求的路径
    void setPath(const std::string& v) { m_path = v;}

    // 设置HTTP请求的查询参数
    void setQuery(const std::string& v) { m_query = v;}

    // 设置HTTP请求的Fragment
    void setFragment(const std::string& v) { m_fragment = v;}

    // 设置HTTP请求的消息体
    void setBody(const std::string& v) { m_body = v;}

    // 是否自动关闭
    bool isClose() const { return m_close;}

    // 设置是否自动关闭
    void setClose(bool v) { m_close = v;}

    // 是否websocket
    bool isWebsocket() const { return m_websocket;}

    // 设置是否websocket
    void setWebsocket(bool v) { m_websocket = v;}

    // 设置HTTP请求的头部MAP
    void setHeaders(const MapType& v) { m_headers = v;}

    // 设置HTTP请求的参数MAP
    void setParams(const MapType& v) { m_params = v;}

    //设置HTTP请求的Cookie MAP
    void setCookies(const MapType& v) { m_cookies = v;}

    //获取HTTP请求的头部参数
    std::string getHeader(const std::string& key, const std::string& def = "") const;

    // 获取HTTP请求的请求参数,如果存在则返回对应值,否则返回默认值
    std::string getParam(const std::string& key, const std::string& def = "");

    // 获取HTTP请求的Cookie参数,如果存在则返回对应值,否则返回默认值
    std::string getCookie(const std::string& key, const std::string& def = "");

    //设置HTTP请求的头部参数
    void setHeader(const std::string& key, const std::string& val);

    // 设置HTTP请求的请求参数
    void setParam(const std::string& key, const std::string& val);

    // 设置HTTP请求的Cookie参数
    void setCookie(const std::string& key, const std::string& val);

    // 删除HTTP请求的头部参数
    void delHeader(const std::string& key);

    // 删除HTTP请求的请求参数
    void delParam(const std::string& key);

    // 删除HTTP请求的Cookie参数
    void delCookie(const std::string& key);

    //判断HTTP请求的头部参数是否存在
    bool hasHeader(const std::string& key, std::string* val = nullptr);

    //判断HTTP请求的请求参数是否存在
    bool hasParam(const std::string& key, std::string* val = nullptr);

    // 判断HTTP请求的Cookie参数是否存在
    bool hasCookie(const std::string& key, std::string* val = nullptr);

    // 检查并获取HTTP请求的头部参数 如果存在且转换成功返回true,否则失败val=def
    template<class T>
    bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T()) 
	{
        return checkGetAs(m_headers, key, val, def);
    }

    // 获取HTTP请求的头部参数,如果存在且转换成功返回对应的值,否则返回def
    template<class T>
    T getHeaderAs(const std::string& key, const T& def = T()) 
	{
        return getAs(m_headers, key, def);
    }

    // 检查并获取HTTP请求的请求参数,如果存在且转换成功返回true,否则失败val=def
    template<class T>
    bool checkGetParamAs(const std::string& key, T& val, const T& def = T()) 
	{
        initQueryParam();
        initBodyParam();
        return checkGetAs(m_params, key, val, def);
    }

    //获取HTTP请求的请求参数,如果存在且转换成功返回对应的值,否则返回d
    template<class T>
    T getParamAs(const std::string& key, const T& def = T()) 
	{
        initQueryParam();
        initBodyParam();
        return getAs(m_params, key, def);
    }

    // 检查并获取HTTP请求的Cookie参数,如果存在且转换成功返回true,否则失败val=def
    template<class T>
    bool checkGetCookieAs(const std::string& key, T& val, const T& def = T()) 
	{
        initCookies();
        return checkGetAs(m_cookies, key, val, def);
    }

    // 获取HTTP请求的Cookie参数
    template<class T>
    T getCookieAs(const std::string& key, const T& def = T()) 
	{
        initCookies();
        return getAs(m_cookies, key, def);
    }

    //序列化输出到流中
    std::ostream& dump(std::ostream& os) const;

    //转成字符串类型
    std::string toString() const;

    void init();
    void initParam();
    void initQueryParam();
    void initBodyParam();
    void initCookies();
private:
	/*
    HTTP请求由请求行、消息报头、请求正文组成
    请求行：请求方法、版本、URL
    URL：
    http://www.asoxasd.com:8080/news/index.asp?boardID=5&ID=24443&page=1#name
    1、协议部分，http
    2、域名部分,www.asoxasd.com
    3、端口部分,8080
    4、虚拟目录,/news/
    5、文件名部分,index.asp
    6、媌部分,从#到最后
    7、参数部分,从？到#

    消息报头： 由一系列的键值对组成，允许客户端向服务端发送一些附加消息或者客户端自身的信息
            Accept  指定客户端能够接收的内容类型
            Accept-Charset     浏览器可以接受的字符编码集
            Accept-Encoding     指定浏览器可以支持的web服务器返回内容压缩编码类型
            Accept-Language     浏览器可以接受的语言
            Accept-Ranges       可以请求网页实体的一个或者多个子范围字段
            Authorization       HTTP授权的授权证书类型
            Cache-Contrrol      指定请求和响应遵循的缓存机制
            Connection          表示是否需要持久连接（HTTP1.1默认1进行持久连接）
            Cookie              HTTP请求发送时，会把保存在该请求域名下的所有cookie值一起发送给web服务器
            Content-Length      请求的内容长度
            Content-Type        请求的与实体对应的MIME信息
    请求正文： 只有发送POST请求时才会有请求正文
    */
    HttpMethod m_method;// HTTP方法
    uint8_t m_version;// HTTP版本
	
	std::string m_path;// 请求路径
    std::string m_query;// 请求参数（URL中的参数字符串，需要将其中的各个参数解析到m_params字典中）
    std::string m_fragment;// 请求fragment
    
    bool m_close;// 是否自动关闭（即持久连接是否开启）
    bool m_websocket;// 是否为websocket

    uint8_t m_parserParamFlag;//记录哪些参数已经初始化了
	
    std::string m_body;// 请求消息体
    MapType m_headers;// 请求头部MAP 即消息报头
    MapType m_params;// 请求参数MAP
    MapType m_cookies;// 请求Cookie MAP
};

// HTTP响应结构体
class HttpResponse {
public:
    /// HTTP响应结构智能指针
    typedef std::shared_ptr<HttpResponse> ptr;
    /// MapType
    typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;
    // 构造函数
    HttpResponse(uint8_t version = 0x11, bool close = true);

    // 返回响应状态
    HttpStatus getStatus() const { return m_status;}

    // 返回响应版本
    uint8_t getVersion() const { return m_version;}

    // 返回响应消息体
    const std::string& getBody() const { return m_body;}

    // 返回响应原因
    const std::string& getReason() const { return m_reason;}

    // 返回响应头部MAP
    const MapType& getHeaders() const { return m_headers;}

    // 设置响应状态
    void setStatus(HttpStatus v) { m_status = v;}

    //设置响应版本
    void setVersion(uint8_t v) { m_version = v;}

    // 设置响应消息体
    void setBody(const std::string& v) { m_body = v;}

    //设置响应原因
    void setReason(const std::string& v) { m_reason = v;}

    // 设置响应头部MAP
    void setHeaders(const MapType& v) { m_headers = v;}

    // 是否自动关闭
    bool isClose() const { return m_close;}

    // 设置是否自动关闭
    void setClose(bool v) { m_close = v;}

    // 是否websocket
    bool isWebsocket() const { return m_websocket;}

    // 设置是否websocket
    void setWebsocket(bool v) { m_websocket = v;}

    // 获取响应头部参数
    std::string getHeader(const std::string& key, const std::string& def = "") const;

    // 设置响应头部参数
    void setHeader(const std::string& key, const std::string& val);

    // 删除响应头部参数
    void delHeader(const std::string& key);

    //检查并获取响应头部参数
    template<class T>
    bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T()) 
	{
        return checkGetAs(m_headers, key, val, def);
    }

    // 获取响应的头部参数
    template<class T>
    T getHeaderAs(const std::string& key, const T& def = T()) 
	{
        return getAs(m_headers, key, def);
    }

    // 序列化输出到流
    std::ostream& dump(std::ostream& os) const;

    // 转成字符串
    std::string toString() const;

    void setRedirect(const std::string& uri);
    void setCookie(const std::string& key, const std::string& val,
                   time_t expired = 0, const std::string& path = "",
                   const std::string& domain = "", bool secure = false);
private:
	/*
响应消息：状态行、消息报头、空行、响应正文
状态行：HTTP协议版本号、状态码、状态消息
    状态码：
        1xx:指示信息--表示请求已接收，继续处理
        2xx:成功--表示请求已被成功接收，理解、接受
        3xx:重定向--要完成请求必须进行更近一步的操作
        4xx:客户端错误--请求有语法错误或者请求无法实现
        5xx:服务端错误--服务器未能实现合法的请求
消息报头：用来说明客户端要使用的一些附加消息
响应正文：服务端返回给客户端的文本信息
*/
    HttpStatus m_status;// 响应状态
    uint8_t m_version;// 版本
    bool m_close;// 是否自动关闭
    bool m_websocket;// 是否为websocket
    std::string m_body;// 响应消息体
    std::string m_reason;// 响应原因
    MapType m_headers;// 响应头部MAP
    std::vector<std::string> m_cookies;
};

//流式输出HttpRequest
std::ostream& operator<<(std::ostream& os, const HttpRequest& req);

//流式输出HttpResponse
std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp);

}
}

#endif