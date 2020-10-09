#ifndef CHIVE_NET_SOCKETOPS_H
#define CHIVE_NET_SOCKETOPS_H

#include "chive/base/clog/chiveLog.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>      // snprintf
#include <sys/uio.h>    // readv

namespace chive
{
namespace net
{
namespace socketops
{
int createNonblockingOrDie(sa_family_t family)
{
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPPROTO_TCP);
    if (sockfd < 0)
    {
        CHIVE_LOG_ERROR("create socket failed! socket %d", sockfd);
    }
    return sockfd;
}

int connect(int sockfd, const struct sockaddr* addr)
{
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(sockaddr)));
}

int bindOrDie(int sockfd, const struct sockaddr* addr)
{
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(sockaddr)));
    if (ret < 0) {
        CHIVE_LOG_ERROR("bind sockfd %d failed, ret %d", sockfd, ret);
    }  
}

void listenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        CHIVE_LOG_ERROR("listen %d failed, ret %d", sockfd, ret);
    }
}
// int accept(int sockfd, struct sockaddr_in6* addr);

} // namespace socketops

} // namespace net

} // namespace chive


#endif