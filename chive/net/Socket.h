#ifndef CHIVE_NET_SOCKET_H
#define CHIVE_NET_SOCKET_H

#include "chive/base/noncopyable.h"

struct tcp_info;    /// #include<netinet/tcp.h> 

namespace chive
{
namespace net
{

class InetAddress;

/**
 * socket fd 的封装
 * --- 线程安全，所有操作都委托给操作系统
 */

class Socket : noncopyable
{
public:
    explicit Socket(int sockfd)
        : sockfd_ (sockfd)
    { }

    /**
     * 移动构造
     */
    Socket(Socket&& sock) noexcept;

    Socket& operator=(Socket&& rhs) noexcept;

    ~Socket();

    /**
     * 设置sockfd_为-1从而不可用
     */
    void setNoneFd();     

    bool isValid() { return sockfd_ >= 0; }

    /**
     * 获取socket fd
     */
    int fd() const { return sockfd_; }

    /**
     * 获取tcp info
     * @param tcpInfo 指针获取tcp_info
     * @return
     */
    bool getTcpInfo(struct tcp_info* tcpInfo) const;

    /**
     * 获取tcp info 字符串形式
     * @param buf 
     * @param len
     * @return 
     */
    bool getTcpInfoString(char* buf, int len) const;

    /**
     * 绑定地址
     * @param localAddr 待绑定的地址对象
     */
    void bindAddress(const InetAddress& localAddr);

    /**
     * 监听sockfd_
     * 地址已被使用时abort
     */
    void listen();

    /**
     * 接收新连接并返回 连接的socket
     * @param peerAddr 获取peer address信息
     * @return success返回sockfd / error返回-1
     */
    int accept(InetAddress* peerAddr);

    /**
     * shutdown 写端
     */
    void shutdownWrite();

    /**
     * enable/disable TCP_NODELAY
     * 禁用 Nagle 算法
     */
    void setTcpNoDelay(bool on);

    /**
     * enable/disable SO_RESUEADDR
     * 设置地址复用
     */
    void setReuseAddr(bool on);

    /**
     * enble/disable SO_REUSEPORT
     * 设置端口复用
     */
    void setReusePort(bool on);

    /**
     * enable/disable SO_KEEPALIVE
     * 设置keep alive 长连接
     */
    void setKeepAlive(bool on);

    /**
     * 获取socket fd 上的errno 错误码
     * @return errno
     */
    int Socket::getSocketError();

private:
    const int sockfd_;
};

} // namespace net

} // namespace chive

#endif