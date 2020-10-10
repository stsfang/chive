#include "chive/net/SocketOps.h"


using namespace chive;
using namespace chive::net;


int socketops::createNonblockingOrDie(sa_family_t family)
{
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        CHIVE_LOG_ERROR("create socket failed! socket %d", sockfd);
    }
    return sockfd;
}

int socketops::connect(int sockfd, const struct sockaddr* addr)
{
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(sockaddr)));
}

int socketops::bindOrDie(int sockfd, const struct sockaddr* addr)
{
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(sockaddr)));
    if (ret < 0) {
        CHIVE_LOG_ERROR("bind sockfd %d failed, ret %d", sockfd, ret);
    }  
}

void socketops::listenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        CHIVE_LOG_ERROR("listen %d failed, ret %d", sockfd, ret);
    }
}
// int accept(int sockfd, struct sockaddr_in6* addr);

void socketops::close(int sockfd)
{
    if (::close(sockfd) < 0)
    {
        CHIVE_LOG_ERROR("error occurs when closing sockfd %d", sockfd);
    }
}

/**
 * 检查sockfd上是否有错误发生
 * @return 返回错误码
 */
int socketops::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else 
    {
        return optval;
    }
}

struct sockaddr* socketops::sockaddr_cast(struct sockaddr_in6* addr)
{
    return static_cast<struct sockaddr*>(static_cast<void*>(addr));
}

struct sockaddr* socketops::sockaddr_cast(struct sockaddr_in* addr)
{
    return static_cast<struct sockaddr*>(static_cast<void*>(addr));
}
const struct sockaddr* socketops::sockaddr_cast(const struct sockaddr_in* addr)
{
    return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
}

struct sockaddr_in6 socketops::getLocalAddr6(int sockfd)
{
    struct sockaddr_in6 localaddr;
    memset(&localaddr, 0, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
    if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
    {
        CHIVE_LOG_ERROR("getsockname at socket %d failed!", sockfd);
    }
    return localaddr;
}

struct sockaddr_in socketops::getLocalAddr4(int sockfd)
{
    struct sockaddr_in localaddr;
    memset(&localaddr, 0, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
    if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
    {
        CHIVE_LOG_ERROR("getsockname at socket %d failed!", sockfd);
    }
    return localaddr;
}

struct sockaddr_in6 socketops::getPeerAddr6(int sockfd)
{
    struct sockaddr_in6 peeraddr;
    memset(&peeraddr, 0, sizeof(peeraddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(peeraddr));
    if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
    {
        CHIVE_LOG_ERROR("getpeername at socket %d failed!", sockfd);
    }
    return peeraddr;
}

struct sockaddr_in socketops::getPeerAddr4(int sockfd)
{
    struct sockaddr_in peeraddr;
    memset(&peeraddr, 0, sizeof(peeraddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(peeraddr));
    if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
    {
        CHIVE_LOG_ERROR("getpeername at socket %d failed!", sockfd);
    }
    return peeraddr;
}

/**
 * 检查是否socket自连接
 * 原理: 检查local和peer端的ip和port是否相同，相同则说明发生了自连接
 */
bool socketops::isSelfConnect(int sockfd)
{
    struct sockaddr_in6 localaddr = getLocalAddr6(sockfd);
    struct sockaddr_in6 peeraddr = getPeerAddr6(sockfd);
    if (localaddr.sin6_family == AF_INET)       // IPv4
    {
        const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
        const struct sockaddr_in* paddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
        return laddr4->sin_port == paddr4->sin_port 
                    && laddr4->sin_addr.s_addr == paddr4->sin_addr.s_addr;
    }
    else if (localaddr.sin6_family == AF_INET6) // IPv6
    {
        // int memcmp (const void *s1, const void *s2, size_t n); -- #include <string.h>
        // 比较s1和s2的前n个字符
        return localaddr.sin6_port == peeraddr.sin6_port
                    && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof(localaddr.sin6_addr)) == 0;
    }
    else 
    {
        CHIVE_LOG_ERROR("Unknown protocol, not support!");
        return false;
    }
}