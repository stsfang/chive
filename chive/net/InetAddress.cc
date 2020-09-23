#include "chive/net/InetAddress.h"
#include "chive/base/clog/chiveLog.h"
// #include "chive/base/Logger.h"

#include <string.h>
#include <arpa/inet.h>

#include <endian.h>             /// htobe16()...

using namespace chive::net;

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };


// static const in_addr_t kInaddrAny = INADDR_ANY;
InetAddress::InetAddress(uint16_t port) 
    : addr_ {}  /*memset(&addr_, sizeof arrd_)*/
{
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = INADDR_ANY;
    addr_.sin_port = htobe16(port); // 主机字节序转网络字节序
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
    : addr_{}
{
    addr_.sin_family = AF_INET;
    addr_.sin_port = htobe16(port);
    if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0)
    {
        CHIVE_LOG_ERROR("inet_pton failed!");
    }
}

std::string InetAddress::toIp() const 
{
    const int size = 32;
    char buf[size];
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(size));
    return buf; // 隐式转换
}

std::string InetAddress::toIpPort() const 
{
    const int size = 32;
    char buf[size];
    // <netinet/in.h>
    // #define INET_ADDRSTRLEN 16 /* for IPv4 dotted-decimal */
    char host[INET_ADDRSTRLEN] = "INVALID";
    ::inet_ntop(AF_INET, &addr_.sin_addr, host, static_cast<socklen_t>(sizeof(host)));

    uint16_t port = be16toh(addr_.sin_port);
    snprintf(buf, size, "%s:%d", host, port);
    return buf;
}


uint64_t InetAddress::toPort() const 
{
    return be16toh(portNetEndian());      // 网络字节序到主机序端口号
}

const sockaddr_in& InetAddress::getSockAddr() const 
{
    return addr_;
}

void InetAddress::setSockAddr(const sockaddr_in& addr)
{
    addr_  = addr;
}

sockaddr_in InetAddress::getLocalAddress(int sockfd)
{
    sockaddr_in localAddr {};
    socklen_t addrLen = sizeof(localAddr);
    if (::getsockname(sockfd, reinterpret_cast<sockaddr*>(&localAddr), 
                                    static_cast<socklen_t*>(&addrLen) ) < 0)
    {
        CHIVE_LOG_ERROR("getsockname failed!");
    }
    return localAddr;
}

sockaddr_in InetAddress::getPeerAddress(int sockfd)
{
    sockaddr_in peerAddr {};
    socklen_t addrLen = sizeof(peerAddr);
    if (::getpeername(sockfd, reinterpret_cast<sockaddr*>(&peerAddr), 
                    static_cast<socklen_t*>(&addrLen)) < 0)
    {
        CHIVE_LOG_ERROR("getpeername failed!");
    }
    return peerAddr;
}