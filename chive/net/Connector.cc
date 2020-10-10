#include "chive/net/Connector.h"
#include "chive/base/clog/chiveLog.h"
#include "chive/net/Channel.h"
#include "chive/net/EventLoop.h"
#include "chive/net/SocketOps.h"

#include <errno.h>

using namespace chive;
using namespace chive::net;

// static member
const int Connector::kMaxRetryDelayMs;

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
    // 发起连接
    int ret = socketops::connect(sockfd, socketops::sockaddr_cast(&serverAddr_.getSockAddr()));
    int savedErrno = (ret == 0) ? 0 : errno;
    ///FIXME: handle errno
    switch (savedErrno)
    {
        case 0:
        case EINPROGRESS: /*Linux 非阻塞connect，EINPROGRESS，正在连接*/
        case EINTR:
        case EISCONN:
            /*stevens书中说明要在connect后，继续判断该socket是否可写,可写则证明连接成功*/
            connecting(sockfd);
            break;
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            CHIVE_LOG_ERROR("connector error, errno %d", savedErrno);
            socketops::close(sockfd);
            break;
        default:
            CHIVE_LOG_ERROR("Unexpected error, errno %d", savedErrno);
            socketops::close(sockfd);
            break;
    }
}

void Connector::restart()
{
    loop_->assertInLoopThread();
    setState(State::kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

/**
 * Linux 非阻塞 socket 
 * 处于正在连接的状态
 * 设置回调，开启可写以检查是否连接成功 （可写事件来到）
 */
void Connector::connecting(int sockfd)
{
    setState(State::kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    ///FIXME: unsafe
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
    ///FIXME: unsafe
    channel_->setErrorCallback(std::bind(&Connector::handleError, this));
    // trigger epoller 监听可写事件，若可写说明连接成功，触发回调 `handleWrite()`
    // handleWrite 检查连接是否真的成功，避免'自连接'或其他错误 遇到Error可尝试retry连接
    channel_->enableWriting();
}

/**
 * 停止channel上事件，移除channel并置空channel
 * NOTED: 为何要放在IO loop线程？
 */
int Connector::removeAndResetChannel()
{
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->getFd();
    
    // must transfer `resetChannel()` to I/O loop
    /// FIXME: unsafe
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

// 销毁channel，置空shared_ptr<Channel> channel_ 
void Connector::resetChannel()
{
    channel_.reset();
}

/**
 * Tcp自连接的问题
 * ref:
 * 1. https://www.jianshu.com/p/fe7383e3f14c
 * 2. https://blog.csdn.net/l101606022/article/details/80100640
 * 3. socket连接和http连接：https://blog.csdn.net/wwd0501/article/details/52412396
 */
void Connector::handleWrite()
{
    CHIVE_LOG_DEBUG("connector now state %d", static_cast<int>(state_));
    if (state_ == State::kConnecting) 
    {
        // 移除channel
        int sockfd = removeAndResetChannel();
        int err = socketops::getSocketError(sockfd);
        if (err) {
            ///FIXME:
            /// #define strerror_r(...) (pthread_testcancel(), strerror_r(__VA_ARGS__))
            CHIVE_LOG_WARN("SO_ERROR = %d", err);
            retry(sockfd);
        } else if (socketops::isSelfConnect(sockfd)) {
            CHIVE_LOG_WARN("self connect, retry connecting at socket %d", sockfd);
            retry(sockfd);
        } else {
            setState(State::kConnected);
            if (connect_) {
                newConnectionCallback_(sockfd);
            } else {
                socketops::close(sockfd);
            }
        }
    }
    else 
    {
        ///FIXME: what happened?
        assert(state_ == State::kDisconnected);
    }
}

void Connector::handleError()
{
    CHIVE_LOG_ERROR("state = %d", static_cast<int>(state_));
    if (state_ == State::kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = socketops::getSocketError(sockfd);
        CHIVE_LOG_ERROR("SO_ERROR = %s", err);
        retry(sockfd);
    }
}

/**
 * 尝试重连server端
 */
void Connector::retry(int sockfd)
{
    // 1. 关闭旧的socket，重新连接
    socketops::close(sockfd);
    setState(State::kDisconnected);
    if (connect_)
    {
        CHIVE_LOG_INFO("retry connecting to %s in %d milliseconds", 
                        serverAddr_.toIpPort(), retryDelayMs_);
        //2. 转移到IO线程进行重连
        loop_->runAfter(retryDelayMs_/1000.0, 
                        std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else 
    {
        CHIVE_LOG_DEBUG("do not connect");
    }
}