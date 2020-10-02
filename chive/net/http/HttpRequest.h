#ifndef CHIVE_NET_HTTP_REQUEST_H
#define CHIVE_NET_HTTP_REQUEST_H

#include "chive/base/copyable.h"
#include <string>
#include <map>

/// ref: https://www.cnblogs.com/MandyCheng/p/11151803.html
/// http报文格式：https://www.cnblogs.com/kageome/p/10859996.html


namespace chive
{
namespace net
{
class HttpRequest : copyable
{
public:
    using Timestamp     = uint64_t;
    using HttpHeader    = std::map<std::string, std::string>;
    using HttpBody      = std::string;

    enum class HttpMethod 
    {
        INVALID, GET, POST, HEAD, PUT, DELETE
    };

    enum class HttpVersion
    {
        kUnknown, kHttp10, kHttp11, kHttp20
    };

    HttpRequest();
    void setVersion(HttpVersion v)
    { version_ = v; }

    HttpVersion version() const 
    { return version_; }

    /**
     * 设置HTTP Method
     * @param start 字符串开始的地址
     * @param end 字符串结束的地址
     * @return true / false
     */
    bool setMethod(const char* start, const char* end);

    HttpMethod method() const 
    { return method_; }

    const char* methodToString() const;

    void setPath(const char* start, const char* end)
    { path_.assign(start, end); }

    const std::string path() const 
    { return path_; }

    void setQuery(const char* start, const char* end)
    { query_.assign(start, end); }

    const std::string query() const 
    { return query_; }

    void setReceiveTime(Timestamp t)
    { receiveTime_ = t; }

    Timestamp receiveTime() const 
    { return receiveTime_; }

    void addHeader(const char* start, const char* colon, const char* end);

    std::string getHeader(const std::string field) const;

    const HttpHeader& getHeaders() const 
    { return headers_; }

    void setBody(const std::string& body)
    { body_ = std::move(body); }

    const std::string getBody() const 
    { return body_; }

    void swap(HttpRequest& that);

    
private:
    HttpVersion version_;
    HttpMethod method_;
    std::string path_;
    std::string query_;
    Timestamp receiveTime_;
    HttpHeader headers_;
    HttpBody body_;

};

} // namespace net
} // namespace chive

#endif