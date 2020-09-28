#ifndef CHIVE_NET_TCPCONNECTION_H
#define CHIVE_NET_TCPCONNECTION_H

#include "chive/base/noncopyable.h"
#include "chive/net/Callbacks.h"
#include "chive/net/InetAddress.h"  // 用到了InetAddress实例
#include "chive/net/Buffer.h"

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

static const int HIGH_WATER_MARK_SIZE = 64 * 1024 * 1024;

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
    bool isDisConnected() const { return state_ == kDisconnected; }

    // -- thread safe
    void send(const std::string& message);
    void send(const void* message, size_t len);
    // -- thread safe
    void shutdown();

    /**
     *  -- TCP keepalive
     * 定期检查TCP连接是否还存在，如果应用层有做‘心跳’的话则不是必须的
     * 但一个通用的网络库应该暴露其接口
     */
    void setKeepAlive(bool on);
    /**
     * -- TCP No Delay 对低延迟网络服务关键
     * 禁用Nagle算法，避免连续发包出现延迟，
     */
    void setTcpNoDelay(bool on);
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
    // ‘低水位回调’
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    // ‘高水位回调'
    void setWaterMarkCallback(const HighWaterMarkCallback& cb) 
    {}
private:
    // TcpConnection 状态 (tcp connection的状态转移)
    // 初始化状态即为 connecting
    enum StateE 
    {
        kConnecting,        // init state
        kConnected,         // connecting -> connected, after connectEstablished()
        kDisconnecting,     // kConnected -> Disconnecting, after shutdown()
        kDisconnected,      // connected/Disconnecting -> disconnected, after handleClose()
    };

    /// FIXME: 需要线程安全吗??
    void setState(StateE s) { state_ = s; }

    // 事件处理
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    /**
     * 在IO线程发送数据给客户端
     * @param message 待发送的消息
     */
    void sendInLoop(const std::string& message);
    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    
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
    /// 数据发送完毕时回调
    WriteCompleteCallback writeCompleteCallback_;
    /// 高水位回调: 发送速率 > 对方接收速率，
    /// 防止数据堆积在本地
    HighWaterMarkCallback highWaterMarkCallback_;
    size_t highWaterMark_;      /// 触发高水位回调的阈值
    Buffer inputBuffer_;        /// 接收数据的缓冲区
    Buffer outputBuffer_;       /// 发送数据的缓冲区
};
} // namespace net

} // namespace chive


#endif