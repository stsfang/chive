#include "chive/net/TcpConnection.h"
#include "chive/net/EventLoop.h"
#include "chive/net/Socket.h"
#include "chive/net/Channel.h"
#include "chive/base/clog/chiveLog.h"
#include <errno.h>

using namespace chive;
using namespace chive::net;
using namespace std::placeholders; 

// declaration in Callbacks.h
void chive::net::defaltConnectionCallback(const TcpConnectionPtr& conn)
{
    CHIVE_LOG_DEBUG("connection localAddr %s peerAddr %s connected state %s", 
                            conn->localAddress().toIpPort().c_str(),
                            conn->peerAddress().toIpPort().c_str(), 
                            (conn->isConnected()? "Up" : "Down"));
}

void chive::net::defaltMessageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
    CHIVE_LOG_DEBUG("connection localAddr %s peerAddr %s connected state %s received data %s", 
                            conn->localAddress().toIpPort().c_str(),
                            conn->peerAddress().toIpPort().c_str(), 
                            (conn->isConnected()? "Up" : "Down"),
                            buf->retrieveAllAsString().c_str());  
}

TcpConnection::TcpConnection(EventLoop* loop, 
                            const std::string& name, 
                            int sockfd, 
                            const InetAddress& localAddr,
                            const InetAddress& peerAddr)
    : loop_ (loop),
      name_ (name),
      state_ (kConnecting),
      socket_ (new Socket(sockfd)),
      channel_ (new Channel(loop, sockfd)),
      localAddr_ (localAddr),
      peerAddr_ (peerAddr),
      highWaterMark_ (HIGH_WATER_MARK_SIZE)

{
    CHIVE_LOG_DEBUG("new connection %p created:  { name: %d, sockfd: %d }", this, name_, sockfd);
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this));
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    CHIVE_LOG_DEBUG("disconnected tcp connection %p with channel %p socket %p fd %d", 
                                this, channel_.get(), socket_.get(), socket_->fd());
}

/// 由channel的handleEvent回调
void TcpConnection::handleRead(Timestamp receiveTime)
{
    CHIVE_LOG_DEBUG("tcp connection %p handle read from connfd %d", 
                        this, channel_->getFd());
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->getFd(), &savedErrno);
    if ( n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else 
    {
        errno = savedErrno;
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting())
    {
        ssize_t n = ::write(socket_->fd(), 
                            outputBuffer_.peek(), 
                            outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(static_cast<size_t>(n));
            if (outputBuffer_.readableBytes() == 0) {
                // 发送完毕，移除可写事件，防止busy-loop
                channel_->disableWriting();     
                // 执行写完的回调
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this()));
                }
                // 正在执行关闭，则调用shutdownInLoop()继续执行关闭过程
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            } else {
                CHIVE_LOG_INFO("tcp connection %p socket %d need write more data",
                                    this, socket_->fd());
            }
        }
        else {
            // int savedErrno = errno;
            /// NOTE: 不需要处理错误
            /// 如果发生n==0,那么handleRead会读到0字节，继而关闭连接 --- from muduo
            /// FIXME: handleRead会读到0字节？
            CHIVE_LOG_ERROR("tcp connection %p write to socket %d error", 
                                    this, socket_->fd());
        }
    }
    else
    {
        CHIVE_LOG_INFO("tcp connection %p is down, no more writing", this);
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    CHIVE_LOG_DEBUG("client disconnected now state %d in connfd %d", 
                        state_, channel_->getFd());
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    // 只是移除fd上全部事件，没有close掉fd, 让Socket对象自己析构
    // 目的:便于定位内存泄露 -- from muduo 
    channel_->disableAll();     
    if (closeCallback_) {
        // 回调 TcpServer::removeConnection
        closeCallback_(shared_from_this());
    }
}

void TcpConnection::handleError()
{
    int err = socket_->getSocketError();
    CHIVE_LOG_ERROR("tcp connection %p [ name: ] - SO_ERROR %d",
                        this, name_.c_str(), err);
}

void TcpConnection::connectEstablished()
{
    CHIVE_LOG_DEBUG("tcp connection %p established connection", this);
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    //将this指针作为shared_ptr<T>指针
    channel_->tie(shared_from_this());
    //向内核事件表注册connection关联的socket
    channel_->enableReading();
    //连接建立完成,回调
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    CHIVE_LOG_DEBUG("tcp connection %p desctroyed", this);
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnected);
    setState(kDisconnected);
    channel_->disableAll();
    
    /// FIXME:
    /// 使用disconnectedCallback 而不是 connectionCallback
    if(connectionCallback_)
        connectionCallback_(shared_from_this());

    loop_->removeChannel(channel_.get());
}


void TcpConnection::send(Buffer* buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        } else {
            /// NOTE: 因为sendInLoop有重载，所以需要指出函数指针类型
            /// ref:https://www.cnblogs.com/and_swordday/p/4643975.html
            void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                std::bind(fp, this, buf->retrieveAllAsString()));
        }
    }
}

///FIXME: need or not?
void TcpConnection::send(const void* message, size_t len)
{
    send(std::string(static_cast<const char*>(message), len));
}

void TcpConnection::send(const std::string& message)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            /// NOTE: 因为sendInLoop有重载，所以需要指出函数指针类型
            /// ref:https://www.cnblogs.com/and_swordday/p/4643975.html
            void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                std::bind(fp, this, message));
        }
    }
}


void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting);   // 关闭写入端
        loop_->runInLoop(
            ///NOTE: 使用shared_from_this() 而不是 this 
            std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
    }
}

void TcpConnection::forceClose()
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->queueInLoop(
            std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        handleClose();
    }
}

void TcpConnection::sendInLoop(const std::string& message)
{
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data,  size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = true;
    // add for give up writing when disconnected.
    if (state_ == kDisconnected) {
        CHIVE_LOG_WARN("disconnected, give up writing");
        return;
    }
    // 1. 先检查outputBuffer_ 有没有未发送的数据, 如无则直接写
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(socket_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_) {
                ///NOTE: 不是runInLoop, 因为发送完毕的回调不需要实时
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                CHIVE_LOG_ERROR("tcpconn %p write to socket %d failed, errno %d",
                            this, channel_->getFd(), errno);
                /// FIXME: any other error??
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    // 如果channel处于写事件状态或者outputBuffer_上有未发送的数据
    // 那么当前数据发送的任务需要排队, 以防数据乱序
    // 解决方案: 开启channel上的写事件
    assert(remaining <= len);
    if (remaining > 0 && !faultError) 
    {
        // 处理高水位回调,防止数据本地堆积
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_)
        {
            loop_->queueInLoop(
                std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}


void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    // 如果channel上没有写事件发生才能shutdown
    // 否则会丢失写事件
    if (!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}

void TcpConnection::setKeepAlive(bool on) 
{ socket_->setKeepAlive(on); }

void TcpConnection::setTcpNoDelay(bool on) 
{ socket_->setTcpNoDelay(on); }

