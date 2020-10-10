#pragma once
#ifndef CHIVE_NET_SOCKETOPS_H
#define CHIVE_NET_SOCKETOPS_H

#include "chive/base/clog/chiveLog.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>      // snprintf
#include <sys/uio.h>    // readv
#include <string.h>
#include <errno.h>

namespace chive
{
namespace net
{
namespace socketops
{
int createNonblockingOrDie(sa_family_t family);

int connect(int sockfd, const struct sockaddr* addr);

int bindOrDie(int sockfd, const struct sockaddr* addr);

void listenOrDie(int sockfd);
// int accept(int sockfd, struct sockaddr_in6* addr);

void close(int sockfd);

/**
 * 检查sockfd上是否有错误发生
 * @return 返回错误码
 */
int getSocketError(int sockfd);

struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
struct sockaddr_in6 getLocalAddr6(int sockfd);

struct sockaddr_in getLocalAddr4(int sockfd);

struct sockaddr_in6 getPeerAddr6(int sockfd);

struct sockaddr_in getPeerAddr4(int sockfd);

/**
 * 检查是否socket自连接
 * 原理: 检查local和peer端的ip和port是否相同，相同则说明发生了自连接
 */
bool isSelfConnect(int sockfd);

} // namespace socketops

} // namespace net

} // namespace chive


#endif