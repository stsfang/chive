#ifndef CHIVE_NET_TCPCONNECTION_H
#define CHIVE_NET_TCPCONNECTION_H

#include "chive/base/noncopyable.h"
#include "chive/net/Callbacks.h"


#include <memory>
#include <string>

namespace chive
{
    
namespace net
{

class Socket;
class Channel;
class EventLoop;
class InetAddress;

class TcpConnection : noncopyale, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop, 
                  const string& name, 
                  int sockfd, 
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool isConnected() const { return state_ == kConnected; }
    // bool isDisConnected() const { return state_ == kDisconnected; }
private:
    // TcpConnection 状态
    // 初始化状态即为 connecting
    enum StateE 
    {
        kConnecting,
        kConnected
    };

    /// FIXME: 需要线程安全吗??
    void setState(StateE s) { state_ = s; }
    void handleRead();

    EventLoop* loop_;
    std::string name_;
    ///FIXME: 状态标志位 用 atomic ??
    StateE state_;              
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress localAddr_;
    InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
};
} // namespace net

} // namespace chive


#endif