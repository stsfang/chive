#include "chive/net/TcpConnection.h"
#include "chive/net/EventLoop.h"
#include "chive/net/Socket.h"
#include "chive/net/InetAddress.h"
#include "chive/net/Channel.h"
#include "chive/base/clog/chiveLog.h"


using namespace chive;
using namespace chive::net;

TcpConnection::TcpConnection(EventLoop* loop, 
                            const string& name, 
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
        std::bind(&TcpConnection::handleRead, this, _1));
    CHIVE_LOG_DEBUG("new connection { name %d sockfd %d }", name_, sockfd);
    socket_->setKeepAlive(true);
}


void TcpConnection::handleRead()
{
    char buf[65536];
    ssize_t n = ::read(channel_->getFd(), buf, sizeof(buf));
    messageCallback_(shared_from_this(), buf, n);
    ///FIXME: close connection if n == 0
}