#include "http_session.h"
#include "http_parser.h"

namespace sylar
{
namespace http
{

HttpSession::HttpSession(Socket::ptr sock, bool owner):SocketStream(sock, owner)
{
}

HttpRequest::ptr HttpSession::recvRequest()
{
    HttpRequestParser::ptr parser(new HttpRequestParser);
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    //uint64_t buff_size = 100;
    std::shared_ptr<char> buffer(new char[buff_size], [](char* ptr){delete[] ptr;});
    char* data = buffer.get();
    int offset = 0;
    do {
        int len = read(data + offset, buff_size - offset);
        if(len <= 0)
        {
            close();
            return nullptr;
        }
        len += offset;//本次读数据长度+上次解析剩余数据长度 = 当前需要解析数据长度
        //返回实际解析的长度
        size_t nparse = parser->execute(data, len);
        if(parser->hasError())
        {
            close();
            return nullptr;
        }
        offset = len - nparse; //本次解析后剩余未解析数据长度
        //读入的数据全部都不再进行解析，数据出错
        if(offset == (int)buff_size)
        {
            close();
            return nullptr;
        }
        if(parser->isFinished())
        {
            break;
        }
    } while(true);
    //解析过程中对于报文正文内容未进行解析保存
    int64_t length = parser->getContentLength();
    if(length > 0)
    {
        std::string body;
        body.resize(length);

        int len = 0;
        //上次读后，解析后的剩余数据长度小于于正文数据长度（则剩余数据全是正文）
        if(length >= offset)
        {
            memcpy(&body[0], data, offset);
            len = offset; //len记录已写正文数据长度
        }
        else
        {
            memcpy(&body[0], data, length);
            len = length;
        }
        length -= offset;
        //length大于0，则表明还有正文数据长度未读入
        if(length > 0)
        {
            if(readFixSize(&body[len], length) <= 0)
            {
                close();
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }

    parser->getData()->init();
    return parser->getData();
}

int HttpSession::sendResponse(HttpResponse::ptr rsp)
{
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return writeFixSize(data.c_str(), data.size());
}

}
}
