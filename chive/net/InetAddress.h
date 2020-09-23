#ifndef CHIVE_NET_INETADDRESS_H
#define CHIVE_NET_INETADDRESS_H


#include <string>

#include "chive/base/copyable.h"
#include <string>
#include <netinet/in.h>



namespace chive
{
namespace net
{

namespace sockets
{
    const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
} // namespace sockets

class InetAddress : chive::copyable
{
public:
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);

    InetAddress(const std::string& ip, uint64_t port, bool ipv6 = false);

    explicit InetAddress(const struct sockaddr_in& addr)
        : addr_ (addr)
    { }

    /**
     * get sa_family
     */
    sa_family_t family() const { return addr_.sin_family; }

    /**
     * get "Ip" string
     */
    std::string toIp() const;
    
    /**
     * get "Ip:port" string
     */
    std::string toIpPort() const;
    
    /**
     * get port number
     */
    uint64_t toPort() const;

    /**
     * 获取sockaddr_in地址信息
     */
    const sockaddr_in& getSockAddr() const;

    /**
     * 设置地址信息
     */
    void setSockAddr(const sockaddr_in& addr);

    /**
     * 获取网络字节序的IP地址
     */
    uint32_t ipNetEndian() const;
    
    /**
     * 获取网络字节序的端口号
     */
    uint16_t portNetEndian() const { return addr_.sin_port; }

    /**
     * 获取本地地址信息
     */
    static sockaddr_in getLocalAddress(int sockfd);

    /**
     * 获取客户端地址信息
     */
    static sockaddr_in getPeerAddress(int sockfd);

    /**
     * 解析主机名到IP地址
     * -- 线程安全的 -- 
     */
    static bool resolve(std::string& hostname, InetAddress* result);


private:
    /// only support IPv4
    struct sockaddr_in addr_;
};
} // namespace net

} // namespace chive

#endif