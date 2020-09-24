#include "chive/net/TcpServer.h"
#include "chive/net/InetAddress.h"
#include "chive/base/clog/chiveLog.h"
#include "chive/net/Acceptor.h"
#include "chive/net/EventLoop.h"

#include <string>
#include <stdio.h>

using namespace chive;
using namespace chive::net;

void defaltConnectionCallback(const TcpConnectionPtr& conn)
{
    CHIVE_LOG_DEBUG("connection localAddr %s peerAddr %s connected state %s", 
                            conn->localAddress().toIpPort().c_str(),
                            conn->peerAddress().toIpPort().c_str(), 
                            (conn->isConnected()? "Up" : "Down"));
}

void defaltMessageCallback(const TcpConnectionPtr& conn, char* buf, int len)
{
    CHIVE_LOG_DEBUG("connection localAddr %s peerAddr %s connected state %s received data %s", 
                            conn->localAddress().toIpPort().c_str(),
                            conn->peerAddress().toIpPort().c_str(), 
                            (conn->isConnected()? "Up" : "Down"),
                            buf);
           
}

TcpServer::TcpServer(EventLoop* loop, InetAddress& listenAddr, const std::string& name, bool reuseport)
    : loop_ (loop),
      name_(name),
      acceptor_ (new Acceptor(loop, listenAddr, reuseport)),
      connectionCallback_(defaltConnectionCallback),
      messageCallback_(defaltMessageCallback),
      started_ (false),
      nextConnId_(1)
{
    CHIVE_LOG_DEBUG("tcpserver %p created", this);
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    CHIVE_LOG_DEBUG("tcpserver quit!");
}

void TcpServer::start()
{
    if (!started_)
    {
        assert(!acceptor_->listening());
        loop_->runInLoop(
            std::bind(&Acceptor::listen, acceptor_.get()));
        CHIVE_LOG_DEBUG("start tcpserver %p eventloop %p acceptor %p", 
                            this, loop_, acceptor_.get());
    }
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
    conn->connectEstablished();
}