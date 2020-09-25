#ifndef CHIVE_NET_TCPCONNECTION_H
#define CHIVE_NET_TCPCONNECTION_H

#include "chive/base/noncopyable.h"
#include "chive/net/Callbacks.h"
#include "chive/net/InetAddress.h"  // 用到了InetAddress实例

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

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop, 
                  const std::string& name, 
                  int sockfd, 
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();

    /**
     * 建立tcp connection的时候调用
     * -- 只允许调用一次
     */
    void connectEstablished();

    /**
     * 移除tcp connection的时候调用
     * -- 只允许调用一次
     */
    void connectDestroyed();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool isConnected() const { return state_ == kConnected; }
    // bool isDisConnected() const { return state_ == kDisconnected; }

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
private:
    // TcpConnection 状态
    // 初始化状态即为 connecting
    enum StateE 
    {
        kConnecting,
        kConnected,
        kDisconnected,
        
    };

    /// FIXME: 需要线程安全吗??
    void setState(StateE s) { state_ = s; }

    // 事件处理
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    
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
    CloseCallback closeCallback_;
};
} // namespace net

} // namespace chive


#endif