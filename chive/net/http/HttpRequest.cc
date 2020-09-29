#include "chive/net/http/HttpRequest.h"
#include "chive/base/clog/chiveLog.h"

#include <assert.h>
#include <stdio.h>
#include <ctype.h>      // isspace()

using namespace chvie::net;

HttpRequest::HttpRequest()
    : method_ (HttpMethod::INVALID),
      version_ (HttpVersion::kUnknown)
{
}

bool HttpRequest::setMethod(const char* start, const char* end)
{
    assert(method_ == HttpMethod::INVALID);

    std::string _m(start, end);
    /// FIXME:
    /// 检查_m是否完整的http type
    switch (_m)
    {
    case "GET":
        method_ = HttpMethod::GET;
        break;
    case "POST":
        method_ = HttpMethod::POST;
        break;
    case "PUT":
        method_ = HTTPMethod::PUT;
        break;
    case "HEAD":
        method_ = HttpMethod::HEAD;
        break;
    case "DELETE":
        method_ = HttpMethod::DELETE;
        break;
    default:
        CHIVE_LOG_ERROR("No such method type!");
        break;
    }
}

const char* HttpRequest::methodToString() const
{
    switch (method_)
    {
    case HttpMethod::GET:
        return "GET";
    case HttpMethod::POST:
        return "POST";
    case HTTPMethod::PUT:
        return "PUT";
    case HttpMethod::HEAD:
        return "HEAD";
    case HttpMethod::DELETE:
        return "DELETE";
    default:
        CHIVE_LOG_ERROR("No such method type!");
        break;
    }  
}

/**
 * #include <ctype.h>
C 库函数 int isspace(int c) 检查所传的字符是否是空白字符。
' '     (0x20)    space (SPC) 空格符
'\t'    (0x09)    horizontal tab (TAB) 水平制表符    
'\n'    (0x0a)    newline (LF) 换行符
'\v'    (0x0b)    vertical tab (VT) 垂直制表符
'\f'    (0x0c)    feed (FF) 换页符
'\r'    (0x0d)    carriage return (CR) 回车符
*/
void HttpRequest::addHeader(const char* start, const char* colon, const char* end)
{
    std::string field(start, colon);
    ++colon;
    while(colon < end && isspace(*colon)) 
    {
        ++colon;
    }
    std::string value(colon, end);
    // 去掉value尾部的空格
    while(!value.empty() && isspace(value[value.size()-1]))
    {
        value.resize(value.size()-1);
    }
    headers_[field] = value;
}

std::string HttpRequest::getHeader(const std::string field) const
{
    std::string value;
    HttpHeader::const_iterator it = headers_.find(field);
    if (it != headers_.end())
    {
        value = it->second;
    }
    return value;
}

void HttpRequest::swap(HttpRequest& that)
{
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    path_.swap(that.path_);
    query_.swap(that.query_);
    std::swap(receiveTime_, that.receiveTime_);
    hreaders_.swap(that.headers_);
}



