#ifndef CHIVE_NET_HTTP_RESPONSE_H
#define CHIVE_NET_HTTP_RESPONSE_H

#include "chive/base/copyable.h"
#include <map>
#include <string>


namespace chive
{
namespace net
{

class Buffer;
class HttpResponse : public copyable
{
public:
    using HttpHeader = std::map<std::string, std::string>;

    enum class HttpStatusCode 
    {
        kUnknown, 
        k200OK = 200, 
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
    };

    explicit HttpResponse(bool close)
        : statusCode_(HttpStatusCode::kUnknown),
          closeConnection_(close)
    {}

    void setStatusCode(HttpStatusCode code)
    { statusCode_ = code; }

    void setStatusMessage(const std::string& message)
    { statusMessage_ = message; }

    void setCloseConnection(bool on)
    { closeConnection_ = on; }

    bool closeConnection() const
    { return closeConnection_; }

    void setContentType(const std::string& contentType)
    { addHeader("Content-Type", contentType); }

    void addHeader(const std::string& key, const std::string& value)
    { headers_[key] = value; }

    void setBody(const std::string& body)
    { body_ = body;}

    void appendToBuffer(Buffer* output) const;

private:
    HttpHeader headers_;
    HttpStatusCode statusCode_;
    /// FIXME: add http version
    std::string statusMessage_;
    bool closeConnection_;
    std::string body_;
};

} // namespace net

} // namespace chive

#endif