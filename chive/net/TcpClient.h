#ifndef CHIVE_NET_TCPCLIENT_H
#define CHIVE_NET_TCPCLIENT_H

#include "chive/base/MutexLock.h"
#include "chive/net/TcpConnection.h"    // <string> <memory>

// #include <string>
namespace chive
{
namespace net
{
class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient : noncopyable 
{
public:
    TcpClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& name);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const 
    {
        MutexLockGuard lock(mutex_);
        return connection_;
    }

    EventLoop* getLoop() const { return loop_; }

    bool retry() const { return retry_; }

    void enableRetry() { retry_ = true; }

    const std::string& name() const { return name_; }

    // Not thead safe
    void setConnectionCallback(ConnectionCallback cb)
    {
        connectionCallback_ = std::move(cb);
    }

    ///FIXME: why do the follow setters should be thread-safe?
    // Not thread safe
    void setMessageCallback(MessageCallback cb)
    {
        messageCallback_ = std::move(cb);
    }

    // Not thread safe
    void setWriteCompleteCallback(WriteCompleteCallback cb)
    {
        writeCompleteCallback_ = std::move(cb);
    }

private:
    // Not thread safe but in IO loop
    void newConnection(int sockfd);
    // Not thread safe but in IO loop
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    ConnectorPtr connector_;    //avoid revealing Connector directly
    const std::string name_;    // tcp client name
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    bool retry_;                // can retry or not
    bool connect_;              // begin connecting
    int nextConnId_;            

    mutable MutexLock mutex_;
    TcpConnectionPtr connection_;   /// guard by mutex_
};
} // namespace net

} // namespace chive


#endif