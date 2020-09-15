///
/// reference to https://github.com/FutaAlice/cpp11logger
/// 

#ifndef CHIVE_BASE_LOGGER_H
#define CHIVE_BASE_LOGGER_H

#include <ctime>
#include <string>
#include <sstream>
#include <fstream>

#include "chive/base/MutexLock.h"
#include "chive/base/noncopyable.h"

/*
#include <ctime> or #include <time.h>

#ifndef _TM_DEFINED
struct tm {
          int tm_sec;       // 秒 – 取值区间为[0,59]
          int tm_min;       // 分 - 取值区间为[0,59] 
          int tm_hour;      // 时 - 取值区间为[0,23] 
          int tm_mday;      // 一个月中的日期 - 取值区间为[1,31] 
          int tm_mon;       // 月份（从一月开始，0代表一月） - 取值区间为[0,11] 
          int tm_year;      // 年份，其值等于实际年份减去1900 
          int tm_wday;      // 星期 – 取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推 
          int tm_yday;      // 从每年的1月1日开始的天数 – 取值区间为[0,365]，其中0代表1月1日，1代表1月2日，以此类推 
          int tm_isdst;     // 夏令时标识符，实行夏令时的时候，tm_isdst为正。不实行夏令时的进候，tm_isdst为0；不了解情况时，tm_isdst()为负。
          };
#define _TM_DEFINED
#endif
*/

/*
struct tm * gmtime(const time_t *timer);    // UTC，世界标准时间，即格林尼治时间                                      
struct tm * localtime(const time_t * timer);    // 本地时间
*/

namespace chive
{
// 日志等级，使用c++11强类型枚举
enum class LogLevel 
{
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

// 抽象基类
class BaseLogger {
// 内部类
class LogStream;

public:
    BaseLogger() = default;
    virtual ~BaseLogger() = default;

    virtual LogStream operator()(LogLevel loglevel = LogLevel::DEBUG);

private:
    MutexLock mutex_;
    tm localtime_{};

    /**
     * 获取本地时间
     */
    const tm* getLocalTime();

    /**
     * @param level 
     * @param message
     */
    void endline(Loglevel level, const std::string& message);

    /**
     * 纯虚函数，打印日志消息
     * @param tmPtr 本地时间
     * @param level 日志等级
     * @param message 消息内容
     */
    virtual void output(const tm* tmPtr, 
                        const std::string& level,
                        const std::string& message) = 0;
};

// BaseLogger::LogStream 内部类的定义
class BaseLogger::LogStream : public std::ostringstream {
public:
    LogStream(BaseLogger& logger, LogLevel level);
    LogStream(const LogStream& other);
    ~LogStream() override;
private:
    BaseLogger& logger_;
    LogLevel level_;
};

// 控制台日志类
class ConsoleLogger : public BaseLogger {
    // c++11特性：继承构造函数
    using BaseLogger::BaseLogger;
    void output(const tm* tmPtr, const std::string& level, const std::string& message) override;
};

// 文档日志类
class FileLogger
    : public BaseLogger,
      private noncopyable {
public:
    explicit FileLogger(std::string filename) noexcept;
    ~FileLogger() override;
private:
    std::ofstream file_;
    void output(const tm* tmPtr, const std::string& level, const std::string& message) override;
};

// 全局实例
extern ConsoleLogger debug;
//extern FileLogger CHIVE_LOG;
} // namespace chive

#endif