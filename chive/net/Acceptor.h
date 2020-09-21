#ifndef CHIVE_NET_ACCEPTOR_H
#define CHIVE_NET_ACCEPTOR_H

#include "chive/base/noncopyable.h"
#include <functional>

namespace chive
{
namespace net
{

class EventLoop;
class InetAddress;

class Acceptor : chive::noncopyable
{
public:
    using NewConnectionCallback = function<void (int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr);
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
    {
        newConnCallback_ = cb;
    }

private:
    
    void handleRead();
    
    EventLoop* loop_;                           /// 
    Socket accepSocket_;                        /// 
    Channel acceptChannel_;                     // 
    NewConnectionCallback newConnCallback_;     /// 回调函数
    bool listening_;                            /// 监听状态

    
};
} // namespace net

} // namespace chive

#endif