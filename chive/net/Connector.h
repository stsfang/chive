#ifndef CHIVE_NET_CONNECTOR_H
#define CHIVE_NET_CONNECTOR_H

#include "chive/base/noncopyable.h"
#include "chive/net/InetAddress.h"

#include <functional>
#include <memory>

namespace chive
{
namespace net
{
class EventLoop;
class Channel;
class Connector : noncopyable,
                  public std::enable_shared_from_this<Connector>
{
public:
    using NewConnectionCallback = std::function<void (int sockfd)>;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }
    
    void start();
    void restart();
    void stop();

    const InetAddress& serverAddress() const 
    { return serverAddr_; }

private:
    enum class State
    {
        kDisconnected,
        kConnecting,
        kConnected,
    };
    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;

    void setState(State s) 
    { state_ = s; }

    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    void resetChannel();
    int removeAndResetChannel();

    EventLoop* loop_;
    InetAddress serverAddr_;
    bool connect_;                      /// 是否允许重连
    ///FIXME: use atomic variable?
    State state_;       
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;
    
};
} // namespace net

} // namespace chie

#endif