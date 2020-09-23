#ifndef CHIVE_NET_TCPSERVER_H
#define CHIVE_NET_TCPSERVER_H

#include "chive/base/noncopyable.h"

#include <map>
#include <memory>
#include <string>

namespace chive
{
namespace net
{
class Acceptor;


class TcpServer : noncopyable 
{
public:
    using ConnectionCallback = std::function<void()>;
    using MessageCallback = std::function<void()>;

    TcpServer(EventLoop* loop, InetAddress& listenAddr);
    ~TcpServer();

    /**
     * 启动server
     * -- 线程安全
     */
    void start();

    // -- 非线程安全 -- 
    void setConnectionCallback(cont ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    // -- 非线程安全 --
    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    /**
     * 新连接到达被accpeted后，创建Tcp Connection,放入ConnectionMap
     * @param sockfd  accepted返回的connfd
     * @param peerAddr 客户端地址信息
     */
    void newConnection(int sockfd, const InetAddress& peerAddr);
    
    EventLoop* loop_;       // the acceptor loop
    const std::string name_;                /// the key of ConnectionMap
    std::unique<Acceptor> acceptor_;        /// avoid revealing/exposing acceptor
    ConnectionCallback connectionCallback_; /// 
    MessageCallback messageCallback_;       /// 
    bool started_;                          /// 
    int nextConnId_;                        /// 
    ConnectionMap connections_;             /// 
};
} // namespace net

} // namespace chive

#endif