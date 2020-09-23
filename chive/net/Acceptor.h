#ifndef CHIVE_NET_ACCEPTOR_H
#define CHIVE_NET_ACCEPTOR_H

#include <functional>
#include "chive/base/noncopyable.h"
#include "chive/net/Channel.h"
#include "chive/net/Socket.h"

namespace chive
{
namespace net
{

class EventLoop;
class InetAddress;

// Acceptor is only seen internally, the users cannot use it directly.
// 
class Acceptor : noncopyable
{
public:
    /// FIXME:
    /// replace sockfd(int) with Socket(obj)
    /// base on movable constructor
    /// using NewConnectionCallback = std::function<void (Socket&& socket, const InetAddress&)>;
    using NewConnectionCallback = std::function<void (int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport = false);
    ~Acceptor();

    /**
     * 获取监听状态
     */
    bool listening() const { return listening_; }

    /**
     * 开始监听
     */
    void listen();

    /**
     * 设置新连接的回调函数
     */
    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnCallback_ = cb; }

private:
    
    void handleRead();
    
    EventLoop* loop_;                           /// 
    Socket acceptSocket_;                        /// 
    Channel acceptChannel_;                     // 
    NewConnectionCallback newConnCallback_;     /// 回调函数
    bool listening_;                            /// 监听状态

    
};
} // namespace net

} // namespace chive

#endif