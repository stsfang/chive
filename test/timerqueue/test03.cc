// copied from muduo/net/tests/TimerQueue_unittest.cc

#include "chive/net/EventLoop.h"

#include <functional>
#include <stdio.h>
#include <iostream>
#include <thread>

using namespace chive;
using namespace chive::net;
using namespace chive::base;
int cnt = 0;
EventLoop* g_loop;

void printTid()
{
  printf("CHIVE pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  printf("CHIVE now %ld\n", Timer::now());
}

void print(const char* msg)
{
  printf("CHIVE msg %ld %s\n", Timer::now(), msg);
  std::cout << "CHIVE print callback is thread id " << CurrentThread::tid() << std::endl;
  if (++cnt == 20)
  {
    g_loop->quit();
  }
}

int main()
{
  printTid();
  EventLoop loop;
  g_loop = &loop;

  print("main");
  //测试不同线程提交
  std::thread t1([&]{
      loop.runAfter(1 * 1000 * 1000, std::bind(print, "once1"));
  }); t1.join();
  loop.runAfter(1.5, std::bind(print, "once1.5"));
  loop.runAfter(2.5 * 1000 * 1000, std::bind(print, "once2.5"));
  loop.runAfter(3.5, std::bind(print, "once3.5"));
  loop.runEvery(2, std::bind(print, "every2"));
  loop.runEvery(3, std::bind(print, "every3"));

  loop.loop();
  print("main loop exits");
  sleep(1);
}
