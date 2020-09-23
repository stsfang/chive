#include <iostream>
#include <thread>

#include "chive/net/EventLoop.h"
using namespace chive::net;

void threadfunc() {
    EventLoop loop;
    loop.loop();
}

int main() {
    std::cout << "start loop" << std::endl;
    chive::net::EventLoop loop;

    std::thread t1(threadfunc);
    t1.join();

    loop.loop();
}