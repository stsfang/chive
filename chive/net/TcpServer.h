#ifndef CHIVE_NET_TCPSERVER_H
#define CHIVE_NET_TCPSERVER_H

#include "chive/base/noncopyable.h"
#include "chive/net/TcpConnection.h"

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
    TcpServer(EventLoop* loop, InetAddress& listenAddr, const std::string& name, bool reuseport = false);
    ~TcpServer();

    /**
     * 启动server
     * -- 线程安全
     */
    void start();

    // -- 非线程安全 -- 
    void setConnectionCallback(const ConnectionCallback& cb);

    // -- 非线程安全 --
    void setMessageCallback(const MessageCallback& cb);

    void setWriteCompleteCallback(const WriteCompleteCallback& cb);

private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    /**
     * 新连接到达被accpeted后，创建Tcp Connection,放入ConnectionMap
     * @param sockfd  accepted返回的connfd
     * @param peerAddr 客户端地址信息
     */
    void newConnection(int sockfd, const InetAddress& peerAddr);
    
    void removeConnection(const TcpConnectionPtr& conn);
    
    EventLoop* loop_;       // the acceptor loop
    const std::string name_;                /// the key of ConnectionMap
    std::unique_ptr<Acceptor> acceptor_;        /// avoid revealing/exposing acceptor
    ConnectionCallback connectionCallback_; /// 
    MessageCallback messageCallback_;       /// 
    WriteCompleteCallback writeCompleteCallback_;
    bool started_;                          /// 
    int nextConnId_;                        /// 
    ConnectionMap connections_;             /// 
};
} // namespace net

} // namespace chive

#endif