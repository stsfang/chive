#include "chive/net/InetAddress.h"
#include "chive/net/Acceptor.h"
#include "chive/net/EventLoop.h"

#include "chive/base/clog/chiveLog.h"

#include <unistd.h>

#include <iostream>

using namespace chive;
using namespace chive::net;


void newConnection(int sockfd, const InetAddress& peerAddr)
{
    std::cout << "newConnection accepted a new connection from "
                << peerAddr.toIpPort().c_str() 
                << std::endl;
    ::write(sockfd, "How are you?\n", 13);
    ::close(sockfd);
}

int main()
{
    std::cout << "main() pid = " << getpid() << std::endl;

    InetAddress listenAddr(9909);
    EventLoop loop;

    Acceptor acceptor(&loop, listenAddr);
    acceptor.setNewConnectionCallback(newConnection);

    acceptor.listen();
    loop.loop();
}