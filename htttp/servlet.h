// Servlet封装

#ifndef __SYLAR_HTTP_SERVLET_H__
#define __SYLAR_HTTP_SERVLET_H__

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include "http.h"
#include "http_session.h"
#include "../util/thread.h"
#include "../util/util.h"

namespace sylar
{
namespace http
{

// Servlet封装
class Servlet
{
public:
    // 智能指针类型定义
    typedef std::shared_ptr<Servlet> ptr;

    /*
     * @brief 构造函数
     * @param[in] name 名称
     */
    Servlet(const std::string& name)
        :m_name(name) {}

    /*
     * @brief 析构函数
     */
    virtual ~Servlet() {}

    /*
     * @brief 处理请求
     * @param[in] request HTTP请求
     * @param[in] response HTTP响应
     * @param[in] session HTTP连接
     * @return 是否处理成功
     */
    virtual int32_t handle(sylar::http::HttpRequest::ptr request, sylar::http::HttpResponse::ptr response, sylar::http::HttpSession::ptr session) = 0;

    // 返回Servlet名称
    const std::string& getName() const { return m_name;}
protected:
    /// 名称
    std::string m_name;
};

// 函数式Servlet
class FunctionServlet : public Servlet
{
public:
    // 智能指针类型定义
    typedef std::shared_ptr<FunctionServlet> ptr;
    // 函数回调类型定义
    typedef std::function<int32_t (sylar::http::HttpRequest::ptr request , sylar::http::HttpResponse::ptr response , sylar::http::HttpSession::ptr session)> callback;


    /*
     * @brief 构造函数
     * @param[in] cb 回调函数
     */
    FunctionServlet(callback cb);
    virtual int32_t handle(sylar::http::HttpRequest::ptr request , sylar::http::HttpResponse::ptr response, sylar::http::HttpSession::ptr session) override;
private:
    // 回调函数
    callback m_cb;
};

// Servlet分发器
class ServletDispatch : public Servlet
{
public:
    // 智能指针类型定义
    typedef std::shared_ptr<ServletDispatch> ptr;
    // 读写锁类型定义
    typedef RWMutex RWMutexType;

    /*
     * @brief 构造函数
     */
    ServletDispatch();

	//决定具体请求使用哪个servlet
    virtual int32_t handle(sylar::http::HttpRequest::ptr request, sylar::http::HttpResponse::ptr response, sylar::http::HttpSession::ptr session) override;

    //添加servlet
    void addServlet(const std::string& uri, Servlet::ptr slt);

    // 添加servlet
    void addServlet(const std::string& uri, FunctionServlet::callback cb);

    // 添加模糊匹配servlet uri 模糊匹配 /sylar_*slt servlet
    void addGlobServlet(const std::string& uri, Servlet::ptr slt);

    //添加模糊匹配servlet
    void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);

    // 删除servlet
    void delServlet(const std::string& uri);

    //删除模糊匹配servlet
    void delGlobServlet(const std::string& uri);

    // 返回默认servlet
    Servlet::ptr getDefault() const { return m_default;}

    // 设置默认servlet
    void setDefault(Servlet::ptr v) { m_default = v;}


    // 通过uri获取servlet
    Servlet::ptr getServlet(const std::string& uri);

    //通过uri获取模糊匹配servlet
    Servlet::ptr getGlobServlet(const std::string& uri);

    // 通过uri获取servlet 优先精准匹配,其次模糊匹配,最后返回默认
    Servlet::ptr getMatchedServlet(const std::string& uri);

private:
    // 读写互斥量
    RWMutexType m_mutex;

    // 精准匹配servlet MAP
    // uri(/sylar/xxx) -> servlet
    std::unordered_map<std::string, Servlet::ptr> m_datas;

    // 模糊匹配servlet 数组
    // uri(/sylar/*) -> servlet
    std::vector<std::pair<std::string, Servlet::ptr> > m_globs;

    // 默认servlet，所有路径都没匹配到时使用
    Servlet::ptr m_default;
};

// NotFoundServlet(默认返回404)
class NotFoundServlet : public Servlet
{
public:
    // 智能指针类型定义
    typedef std::shared_ptr<NotFoundServlet> ptr;

    // 构造函数
    NotFoundServlet(const std::string& name);
    virtual int32_t handle(sylar::http::HttpRequest::ptr request,sylar::http::HttpResponse::ptr response,sylar::http::HttpSession::ptr session) override;

private:
    std::string m_name;
    std::string m_content;
};

}
}

#endif
