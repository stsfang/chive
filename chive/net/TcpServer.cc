#include "chive/net/TcpServer.h"
#include "chive/net/InetAddress.h"
#include "chive/base/clog/chiveLog.h"
#include "chive/net/Acceptor.h"
#include "chive/net/EventLoop.h"


#include <string>
#include <stdio.h>

using namespace chive;
using namespace chive::net;
using namespace std::placeholders;        // function对象占位符

#ifdef CHIVE_IGNORE_SIGPIPE
// 全局对象定义, 忽略SIGPIPE
IgnoreSigPipe initObj;
#endif


TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name, bool reuseport)
    : loop_ (loop),
      name_(name),
      ipPort_(listenAddr.toIpPort()),
      acceptor_ (new Acceptor(loop, listenAddr, reuseport)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      started_ (false),
      nextConnId_(1),
      threadPool_(new EventLoopThreadPool(loop, "chive_evtloopthreadpool#"+name))
{
    CHIVE_LOG_DEBUG("created tcpserver %p", this);
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    CHIVE_LOG_DEBUG("tcpserver quit!");
}

void TcpServer::start()
{
    if (!started_)
    {
        CHIVE_LOG_DEBUG("start tcpserver %p acceptor %p listenfd run in eventloop %p", 
                            this, acceptor_.get(), loop_);
        assert(!acceptor_->listening());
        // 在IO线程进行监听
        loop_->runInLoop(
            std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::setConnectionCallback(const ConnectionCallback& cb)
{
    CHIVE_LOG_DEBUG("set connection callback for tcpserver %p", this);
    connectionCallback_ = cb; 
}

void TcpServer::setMessageCallback(const MessageCallback& cb)
{
    CHIVE_LOG_DEBUG("set message callback for tcpserver %p", this);
    messageCallback_ = cb;
}

void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback& cb)
{
    CHIVE_LOG_DEBUG("set write complete callback for tcpserver %p", this);
    writeCompleteCallback_ = cb;  
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    CHIVE_LOG_INFO("new connection is coming: { connName: %s from peerAddr %s }",
                            connName.c_str(), peerAddr.toIpPort().c_str());
    
    InetAddress localAddr(InetAddress::getLocalAddress(sockfd));

    TcpConnectionPtr conn(
        new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, _1));
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectInLoop, this, conn));
}
// 保证回调在conn的ioloop进行
void TcpServer::removeConnectInLoop(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    CHIVE_LOG_INFO("remove connection %p named %d", 
                    conn.get(), conn->name().c_str());
    size_t n = connections_.erase(conn->name());
    assert(n == 1); (void)n;    
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
}