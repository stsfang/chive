#ifndef CHIVE_NET_HTTP_HTTPSERVER_H
#define CHIVE_NET_HTTP_HTTPSERVER_H

#include "chive/net/TcpServer.h"
#include "chive/base/noncopyable.h"
#include <string>
#include <functional>

namespace chive
{
namespace net
{

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable 
{
public:
    using HttpCallback = std::function<void (const HttpRequest&, HttpResponse*)>;

    HttpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const std::string& name,
               bool reuseport = false);

    EventLoop* getLoop() const
    { return server_.getLoop(); }

    void setHttpCallback(const HttpCallback& cb)
    { httpCallback_ = cb; }

    void setThreadNum(int numThreads)
    { server_.setThreadNum(numThreads); }

    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);
    void onRequest(const TcpConnectionPtr&, const HttpRequest&);

    TcpServer server_;
    HttpCallback httpCallback_;
};

} // namespace net

} // namespace chive

#endif