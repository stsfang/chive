#include "chive/net/EventLoop.h"
#include "chive/net/EventLoopThread.h"
#include "chive/base/clog/chiveLog.h"



void runInThread()
{
    CHIVE_LOG_DEBUG("run in thread...")
}

int main() 
{
    startLogPrint(nullptr);
    CHIVE_LOG_DEBUG("main() begin...");

    chive::net::EventLoopThread loopThread;
    chive::net::EventLoop* loop = loopThread.startLoop();
     CHIVE_LOG_DEBUG("run after...");
    loop->runAfter(2, runInThread);
    loop->runInLoop(runInThread);
    // sleep(1);
    
    sleep(3);
    loop->quit();

    CHIVE_LOG_DEBUG("exit main");
    
}