#include "chive/net/Socket.h"
#include "chive/base/clog/chiveLog.h"
#include "chive/net/InetAddress.h"
#include "chive/base/Logger.h"

#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>        /// srtuct tcp_info
#include <errno.h>
#include <sys/socket.h>
#include <stdio.h>      /// snprintf
#include <string.h>
#include <arpa/inet.h>


using namespace chive;
using namespace chive::net;

Socket::~Socket()
{
    if(isValid())   /// sockfd_ >= 0才能close
    {
        CHIVE_LOG_DEBUG("close socket fd %d", sockfd_);
        ::close(sockfd_);
    }
}

Socket::Socket(Socket &&socket) noexcept 
    : sockfd_ (socket.sockfd_)
{
    socket.setNoneFd();
}

Socket& Socket::operator=(Socket&& rhs) noexcept
{
    if (this != &rhs)
    {
        sockfd_ = rhs.fd();
        rhs.setNoneFd();
    }
    return *this;
}

void Socket::setNoneFd()
{
    sockfd_ = -1;
}

/*
#include <sys/types.h>   
#include <sys/socket.h>
// 获取/设置socket状态
int getsockopt(int s, int level, int optname, void* optval, 
                    socklen_t* optlen);
int setsockopt( int socket, int level, int option_name,
                    const void *option_value, size_t option_len);
*/
bool Socket::getTcpInfo(struct tcp_info* tcpInfo) const 
{
    socklen_t len = sizeof(*tcpInfo);
    memset(tcpInfo, 0, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpInfo, &len) == 0;
}

bool Socket::getTcpInfoString(char* buf, int len) const
{
    struct tcp_info tcpi;
    bool ok = getTcpInfo(&tcpi);
    if (ok)
    {
        snprintf(buf, len, "unrecovered=%u "
                "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                "lost=%u retrans=%u rtt=%u rttvar=%u "
                "sshthresh=%u cwnd=%u total_retrans=%u",
                tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
                tcpi.tcpi_rto,          // Retransmit timeout in usec
                tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
                tcpi.tcpi_snd_mss,
                tcpi.tcpi_rcv_mss,
                tcpi.tcpi_lost,         // Lost packets
                tcpi.tcpi_retrans,      // Retransmitted packets out
                tcpi.tcpi_rtt,          // Smoothed round trip time in usec
                tcpi.tcpi_rttvar,       // Medium deviation
                tcpi.tcpi_snd_ssthresh,
                tcpi.tcpi_snd_cwnd,
                tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
    }
    return ok;
}

void Socket::bindAddress(const InetAddress& localAddr)
{
    sockaddr_in address = localAddr.getSockAddr();
    int ret = ::bind(sockfd_, reinterpret_cast<sockaddr*>(&address), sizeof(address));
    if (ret < 0)
    {
        CHIVE_LOG_ERROR("bind address failed!");
    }
}

void  Socket::listen()
{
    CHIVE_LOG_DEBUG("socket %d begin listening...", sockfd_);
    // SOMAXCONN 内核参数， 默认128，可调优
    if(::listen(sockfd_, SOMAXCONN) < 0)
    {
        CHIVE_LOG_ERROR("listen at %d failed!", sockfd_);
    }
}

int Socket::accept(InetAddress* peerAddr)
{
    sockaddr_in address {};
    auto addressLen = static_cast<socklen_t>(sizeof(address));
    /// ::accept4 非标准扩展,#include <sys/socket.h>
    /// 4th param 设置操作类型
    int connfd = ::accept4(sockfd_, reinterpret_cast<sockaddr*>(&address),
                            &addressLen, SOCK_NONBLOCK|SOCK_CLOEXEC);
    if(connfd >= 0)
    {
        peerAddr->setSockAddr(address);
        CHIVE_LOG_DEBUG("socket %d accept new connection fd %d", sockfd_, connfd);
        return connfd;
    }
    else 
    {
        int savedErrno = errno;
        CHIVE_LOG_ERROR("::accept4 failed, errno %d", savedErrno);
        switch (savedErrno) 
        {
            case EAGAIN:
                CHIVE_LOG_DEBUG("EAGAIN");
                break;
            case ECONNABORTED:
            case EINTR:
            case EPROTO:        // ???
            case EPERM:
            case EMFILE:        // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                CHIVE_LOG_ERROR("unexpected error of ::accept4, errno %d", savedErrno);
                break;
            default:
                CHIVE_LOG_ERROR("unknown error of ::accept4, errno %d",savedErrno);
                break;
        }
    return -1;
    }   // end else 
}

/*
noted   ::close 和 ::shutdown 的区别
#include<sys/socket.h>
int shutdown(int sockfd,int howto);  //返回成功为0，出错为-1
SHUT_RD, 值为 0，关闭连接的读端
SHUR_WR, 值为 1, 关闭连接的写端
SHUT_RDWR, 值为2， 关闭连接的读写两端
*/
void Socket::shutdownWrite()
{   // 关闭连接的写端
    if(shutdown(sockfd_, SHUT_WR) < 0)
    {
        CHIVE_LOG_ERROR("shutdown fd %d failed!", sockfd_);
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int opt = on? 1 : 0;
    if (::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, 
                &opt, static_cast<socklen_t>( sizeof(opt) ) ) < 0 )
    {
        CHIVE_LOG_ERROR("::setsockopt on sockfd %d failed!", sockfd_);
    }
}

void Socket::setReuseAddr(bool on)
{
    int opt = on? 1 : 0;
    if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                            &opt, static_cast<socklen_t>( sizeof(opt) ) ) < 0 )
    {
        CHIVE_LOG_ERROR("::setsockopt on sockfd %d failed!", sockfd_);
    }
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
    int opt = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                            &opt, static_cast<socklen_t>( sizeof(opt) ) );
    if (ret < 0 && on)
    {
        CHIVE_LOG_ERROR("::setsockopt set SO_REUSEPORT failed.");
    }
#else
    if (on)
    {
        CHIVE_LOG_ERROR("SO_REUSEPORT is not supported.");
    }
#endif
}

void Socket::setKeepAlive(bool on)
{
  int opt = on ? 1 : 0;
  if (::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
               &opt, static_cast<socklen_t>( sizeof(opt) ) ) < 0)
    {
        CHIVE_LOG_ERROR("::setsockopt on sockfd %d failed!", sockfd_);
    }
}

int Socket::getSocketError() {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if (::getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) 
    {
        CHIVE_LOG_ERROR("::getsockopt on sockfd %d failed!", sockfd_);
        return errno;
    } 
    else 
    {
        return optval;
    }
}