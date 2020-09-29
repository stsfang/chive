#ifndef CHIVE_NET_HTTP_CONTEXT_H
#define CHIVE_NET_HTTP_CONTEXT_H

#include "chive/base/copyable.h"
#include "chive/net/http/HttpRequest.h"


namespace chive
{
namespace net
{
class Buffer;

class HttpContext : copyable 
{
public:
    using Timestamp = unit64_t;

    enum class HttpParseState
    {
        kExpectRequestLine,     /// 解析请求行
        kExpectHeader,          /// 解析请求头部
        kExpectBody,            /// 解析请求体
        kGotAll                 /// 完成解析
    };

    HttpContext();
    bool parseRequest(Buffer* buf, Timestamp receiveTime);
    bool gotAll() const 
    { return state_ == HttpParseState::kGotAll; }

    void reset();

    const HttpRequest& request() const 
    { return request_; }

    HttpRequest& request()
    { return request_; }

private:
    bool processRequestLine(const char* begin, const char* end);
    HttpParseState state_;
    HttpRequest request_;
};

} // namespace net

} // namespace chive


#endif