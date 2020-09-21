#include "chive/net/Acceptor.h"

Acceptor::Acceptor(EvenpLoop* loop, const InetAddress& listenAddr)
    : loop_ (loop),
      accepSocket_ (),
      accepChannel_ (loop, acceptSocket_.getFd()),
      listening_ (false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading():
}
