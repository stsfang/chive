#include "chive/net/Acceptor.h"
#include "chive/net/EventLoop.h"
#include "chive/net/InetAddress.h"
#include "chive/base/clog/chiveLog.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>

using namespace chive;
using namespace chive::net;



int createNonblocking()
{
    int sockfd = ::socket(AF_INET, 
                SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        CHIVE_LOG_ERROR("::socket create failed! sockfd %d", sockfd);
    }
    return sockfd;
}


Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_ (loop),
      acceptSocket_ (createNonblocking()),
      acceptChannel_ (loop, acceptSocket_.fd()),
      listening_ (false)
{
    CHIVE_LOG_DEBUG("created acceptor %p with socket %d channel %p in eventloop %p",  
                     this, acceptSocket_.fd(), &acceptChannel_, loop);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}


void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    loop_->assertInLoopThread();

    // 接收客户端连接，获取客户端信息
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        std::string hostport = peerAddr.toIpPort();
        CHIVE_LOG_DEBUG("listening socket %d accepts client addr %s in connfd %d", 
                                    acceptSocket_.fd(), hostport.c_str(), connfd);

        if (newConnCallback_)
        {
            newConnCallback_(connfd, peerAddr);     /// 回调
        }
        else 
        {
            ::close(connfd);
        }
    }
    else 
    {
        CHIVE_LOG_ERROR("accept failed, connfd %d", connfd);
        ///FIXME: need extra operations??
    }
}