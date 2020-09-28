#ifndef CHIVE_NET_TCPSERVER_H
#define CHIVE_NET_TCPSERVER_H

#define CHIVE_IGNORE_SIGPIPE
#include "chive/base/noncopyable.h"
#include "chive/net/TcpConnection.h"
#include "chive/net/EventLoopThreadPool.h"

#include <map>
#include <memory>
#include <string>

#ifdef CHIVE_IGNORE_SIGPIPE
#include "chive/net/IgnoreSigPipe.h"    // init global IgnoreSigPipe
#endif

namespace chive
{
namespace net
{
class Acceptor;


class TcpServer : noncopyable 
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    
    TcpServer(EventLoop* loop, InetAddress& listenAddr, const std::string& name, bool reuseport = false);
    ~TcpServer();

    /**
     * 启动server
     * -- 线程安全
     */
    void start();

    /**
     * 设置threadpool的线程个数
     * 必须在start()前调用
     * @param numThreads 
     */
    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb)
    { threadInitCallback_ = cb; }

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
    void removeConnectInLoop(const TcpConnectionPtr& conn);
    
    EventLoop* loop_;       // the acceptor loop
    const std::string name_;                /// the key of ConnectionMap
    std::unique_ptr<Acceptor> acceptor_;        /// avoid revealing/exposing acceptor
    ConnectionCallback connectionCallback_; /// 
    MessageCallback messageCallback_;       /// 
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;
    bool started_;                          /// 
    int nextConnId_;                        /// 
    ConnectionMap connections_;             /// 
    std::unique_ptr<EventLoopThreadPool> threadPool_;
};
} // namespace net

} // namespace chive

#endif