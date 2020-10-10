#include "chive/net/TcpClient.h"
#include "chive/net/Connector.h"
#include "chive/net/EventLoop.h"
#include "chive/net/SocketOps.h"

#include "chive/base/clog/chiveLog.h"
#include <stdio.h>  // snprintf
#include <functional>


using namespace chive;
using namespace chive::net;
using namespace std::placeholders;

namespace chive
{
namespace net
{
namespace cb
{
void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
    loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector)
{
    CHIVE_LOG_DEBUG("it's time to remove connector %p", connector.get());
}
} // namespace cb

} // namespace net

} // namespace chive

TcpClient::TcpClient(EventLoop* loop, 
                    const InetAddress& serverAddr, 
                    const std::string& name)
  : loop_(loop),
    connector_(new Connector(loop, serverAddr)),
    name_(name),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    retry_(false),
    connect_(false),
    nextConnId_(1)
{
    connector_->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, _1));
    CHIVE_LOG_INFO("TcpClient %s - connector %p", name_.c_str(), connector_.get());
}

/**
 * TcpClient销毁，保证TcpConnection安全关闭
 */
TcpClient::~TcpClient()
{
    CHIVE_LOG_INFO("TcpClient %s - connector %p", name_.c_str(), connector_.get());
    TcpConnectionPtr conn;
    bool unique = false;
    {
        MutexLockGuard lock(mutex_);
        unique = connection_.unique();  // connection_.use_count() == 1
        conn = connection_;
    }
    if(conn)
    {
        assert(loop_ == conn->getLoop());
        CloseCallback callback = std::bind(&cb::removeConnection, loop_, _1);
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, callback));
        if (unique) /*只有一个引用，可强制关闭连接*/
        {
            conn->forceClose();
        }
    }
    else
    {
        // connection_ 已关闭，connector_可以安全地移除
        connector_->stop();
        loop_->runAfter(1, std::bind(&cb::removeConnector, connector_));
    }
}

void TcpClient::connect()
{
    CHIVE_LOG_INFO("TcpClient %s - connecting to %s", 
            name_.c_str(), connector_->serverAddress().toIpPort().c_str());
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect()
{
    connect_ = false;

    {
        MutexLockGuard lock(mutex_);
        // 单边断开，关闭写端
        if (connection_)
        {
            connection_->shutdown();  
        }
    }
}

void TcpClient::stop()
{
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
    loop_->assertInLoopThread();
    InetAddress peerAddr(socketops::getPeerAddr4(sockfd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", 
                    peerAddr.toIpPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    InetAddress localAddr(socketops::getLocalAddr4(sockfd));
    TcpConnectionPtr conn(new TcpConnection(loop_, 
                                            connName, 
                                            sockfd, 
                                            localAddr, 
                                            peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, _1));
    {
        MutexLockGuard lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

// 与 newConnection配对操作
// 保证在removeConnection之后不再有访问connection_的方法被调用
void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        MutexLockGuard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if (retry_ && connect_)
    {
        CHIVE_LOG_INFO("TcpClient [ %s ] reconnects to %s", 
                        name_.c_str(), connector_->serverAddress().toIpPort().c_str());
        connector_->restart();
    }
}