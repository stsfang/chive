#include "chive/net/http/HttpServer.h"
#include "chive/net/http/HttpRequest.h"
#include "chive/net/http/HttpResponse.h"
#include "chive/net/http/HttpContext.h"
#include "chive/base/clog/chiveLog.h"

#include <any>

using namespace chive;
using namespace chive::net;
using namespace std::placeholders;

namespace chive
{
namespace net
{
namespace cb
{
void dfaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
    resp->setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

} // namespace cb
} // namespace net
} // namespace chive

HttpServer::HttpServer(EventLoop* loop, 
                       const InetAddress& listenAddr,
                       const std::string& name,
                       bool reuseport)
    : server_(loop, listenAddr, name, reuseport),
      httpCallback_(cb::dfaultHttpCallback)
{
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

void HttpServer::start()
{
    CHIVE_LOG_WARN("HttpServer [ %s ] starts listening on %s", server_.name().c_str(), server_.ipPort().c_str());
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->isConnected())
    {
        conn->setContext(HttpContext());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn,
                            Buffer* buf,
                            Timestamp receiveTime)
{
    CHIVE_LOG_DEBUG("got buffer %s", buf->toString().c_str());
    HttpContext* context = std::any_cast<HttpContext>(conn->getMutableContext());

    if (!context->parseRequest(buf, receiveTime))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context->gotAll())
    {
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
    const std::string& connection = req.getHeader("Connection");
    bool close = connection == "close" ||
                    (req.version() == HttpRequest::HttpVersion::kHttp10 && connection != "Keep-Alive");
    HttpResponse resp(close);
    httpCallback_(req, &resp);      // 处理request，填充response
    Buffer buf;
    resp.appendToBuffer(&buf);      // 将response内容填充到buffer，发送给client
    conn->send(&buf);

    if (resp.closeConnection())     // 短链接，写完则关闭连接
    {
        conn->shutdown();
    }

}