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
    CHIVE_LOG_DEBUG("tcp connection %p closed", this);
}

void TcpConnection::handleRead()
{
    CHIVE_LOG_DEBUG("tcp connection %p handle read", this);
    char buf[65536];
    ssize_t n = ::read(channel_->getFd(), buf, sizeof(buf));
    messageCallback_(shared_from_this(), buf, n);
    ///FIXME: close connection if n == 0
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