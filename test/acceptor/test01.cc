#include "chive/net/InetAddress.h"
#include "chive/net/Acceptor.h"
#include "chive/net/EventLoop.h"

#include "chive/base/clog/chiveLog.h"

#include <unistd.h>

#include <iostream>

using namespace chive;
using namespace chive::net;


void newConnection1(int sockfd, const InetAddress& peerAddr)
{
    CHIVE_LOG_INFO("newConnection accepted a new connection from %s",
                                peerAddr.toIpPort().c_str());
    const char* msg = "How are you?\n";
    ::write(sockfd, msg, sizeof(msg));
    ::close(sockfd);
}

void newConnection2(int sockfd, const InetAddress& peerAddr)
{
    CHIVE_LOG_INFO("newConnection accepted a new connection from %s",
                                peerAddr.toIpPort().c_str());
    const char* msg = "Are you ok?\n";
    ::write(sockfd, msg, sizeof(msg));
    ::close(sockfd);
}

int main()
{
    CHIVE_LOG_INFO("main() pid = %d", getpid());
    startLogPrint(NULL); // 
    CHIVE_LOG_INFO("BEGIN");

    // port 1
    InetAddress listenAddr1(9909);
    // port 2
    InetAddress listenAddr2(9908);

    EventLoop loop;

    Acceptor acceptor1(&loop, listenAddr1);
    Acceptor acceptor2(&loop, listenAddr2);
    acceptor1.setNewConnectionCallback(newConnection1);
    acceptor2.setNewConnectionCallback(newConnection2);
    acceptor1.listen();
    acceptor2.listen();
    loop.loop();
}