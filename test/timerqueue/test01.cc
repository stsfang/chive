#include "chive/net/EventLoop.h"
#include <thread>

using namespace chive::net;
using namespace std;

EventLoop* g_loop;

// test abortNotInLoopThread
void threadfunc() {
    g_loop->loop();
}

int main() {
    EventLoop loop;
    g_loop = &loop;
    thread t(threadfunc);
    t.join();
}