#ifndef V_CURRENTTHREAD_H
#define V_CURRENTTHREAD_H

#include <unistd.h>
#include <type_traits>
namespace chive
{
namespace CurrentThread {
    // 使用extern告诉编译器这些__thread修饰的变量定义在
    // 另一个文件，只能初始化为编译器常量
    extern __thread int t_cachedTid;
    extern __thread char t_tidString[32];
    extern __thread int t_tidStringLength;
    extern __thread const char* t_threadName;

    void cachedTid();

    inline int tid() {
        // __builtin_expect是GCC的內建函数
        //作用：编译器分支预测，减少跳转指令，从而优化性能    
        if(__builtin_expect(t_cachedTid == 0, 0)) {
            cachedTid();
        }
        return t_cachedTid;
    }

    inline int tidStringLength() {
        return t_tidStringLength;
    }
    // 未实现设置线程名
    inline const char* name() {
        return t_threadName;
    }
    //根据主线程的pid == tid
    inline bool isMainThread() {
        return tid() == ::getpid();
    }
};
} // namespace chive

#endif