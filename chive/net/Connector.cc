#include "chive/net/Connector.h"
#include "chive/base/clog/chiveLog.h"
#include "chive/net/Channel.h"
#include "chive/net/EventLoop.h"
#include "chive/net/SocketOps.h"

#include <errno.h>

using namespace chive;
using namespace chive::net;

// static member
// const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(State::kDisconnected),
      retryDelayMs_(kInitRetryDelayMs)
{
    CHIVE_LOG_DEBUG("ctor %p", this);
}

Connector::~Connector()
{
    CHIVE_LOG_DEBUG("dtor %p", this);
    assert(!channel_);  //保证channel已先销毁
}

void Connector::start()
{
    connect_ = true;
    ///FIXME: unsafe
    loop_->runInLoop(
        std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop()
{
    loop_->assertInLoopThread();
    assert(state_ == State::kDisconnected);
    if (connect_) {
        connect();
    } else {
        CHIVE_LOG_DEBUG("do not connect");
    }
}

void Connector::stop()
{
    connect_ = false;
    ///FIXME: unsafe
    loop_->queueInLoop(
        std::bind(&Connector::stopInLoop, this));
}

void Connector::stopInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == State::kConnecting)
    {
        setState(State::kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = socketops::createNonblockingOrDie(serverAddr_.family());
    int ret = socketops::connect(sockfd, serverAddr_.getSockAddr());
    int savedErrno = (ret == 0) ? 0 : errno;
    ///FIXME: handle errno
}

