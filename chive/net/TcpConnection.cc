#include "chive/net/TcpConnection.h"
#include "chive/net/EventLoop.h"
#include "chive/net/Socket.h"
#include "chive/net/Channel.h"
#include "chive/base/clog/chiveLog.h"


using namespace chive;
using namespace chive::net;

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
      peerAddr_ (peerAddr)

{
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this));
    CHIVE_LOG_DEBUG("new connection %p created:  { name: %d, sockfd: %d }", this, name_, sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    CHIVE_LOG_DEBUG("disconnected tcp connection %p with channel %p socket %p fd %d", 
                                this, channel_.get(), socket_.get(), socket_->fd());
}

void TcpConnection::handleRead()
{
    CHIVE_LOG_DEBUG("tcp connection %p handle read from connfd %d", 
                        this, channel_->getFd());
    char buf[65536];
    ssize_t n = ::read(channel_->getFd(), buf, sizeof(buf));
    if ( n > 0)
    {
        messageCallback_(shared_from_this(), buf, n);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else 
    {
        handleError();
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    CHIVE_LOG_DEBUG("client disconnected now state %d in connfd %d", 
                        state_, channel_->getFd());
    assert(state_ == kConnected);
    // 只是移除fd上全部事件，没有close掉fd
    // 目的:便于发现泄露 -- from muduo 
    channel_->disableAll();     
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
    int err = socket_->getSocketError();
    CHIVE_LOG_ERROR("tcp connection %p [ name: ] - SO_ERROR %d",
                        this, name_.c_str(), err);
}

void TcpConnection::connectEstablished()
{
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);
  //将this指针作为shared_ptr<T>指针
  channel_->tie(shared_from_this());
  //向内核事件表注册connection关联的socket
  channel_->enableReading();
  //连接建立完成,回调
  connectionCallback_(shared_from_this());
  CHIVE_LOG_DEBUG("tcp connection %p established connection", this);
}

void TcpConnection::connectDestroyed()
{
    CHIVE_LOG_DEBUG("tcp connection %p desctroyed", this);
    loop_->assertInLoopThread();
    assert(state_ == kConnected);
    setState(kDisconnected);
    channel_->disableAll();
    
    connectionCallback_(shared_from_this());

    loop_->removeChannel(channel_.get());
}