#include "chive/base/Logger.h"

#include <cassert>
#include <chrono>
#include <map>
#include <iostream>
#include <iomanip>
#include <regex>

using namespace chive;

ConsoleLogger chive::debug;

// 日志级别string信息
static const std::map<LogLevel, const std::string> LogLevelInfo =
{
    {LogLevel::TRACE,   "TRACE"},
    {LogLevel::DEBUG,   "DEBUG"},
    {LogLevel::INFO,    "INFO"},
    {LogLevel::WARN,    "WARN"},
    {LogLevel::ERROR,   "ERROR"},
    {LogLevel::FATAL,   "FATAL"}
};

std::ostream& operator<<(std::ostream& stream, const tm* tmStr) {
    return stream << 1900 + tmStr->tm_year << '-'
                  << std::setfill('0') << std::setw(2) << tmStr->tm_mon + 1 << '-'
                  << std::setfill('0') << std::setw(2) << tmStr->tm_mday << ' '
                  << std::setfill('0') << std::setw(2) << tmStr->tm_hour << ':'
                  << std::setfill('0') << std::setw(2) << tmStr->tm_min << ':'
                  << std::setfill('0') << std::setw(2) << tmStr->tm_sec;
}

BaseLogger::LogStream BaseLogger::operator()(LogLevel level) {
    return LogStream(*this, level);
}

const tm* BaseLogger::getLocalTime() {
    auto now = std::chrono::system_clock::now();
    auto inTime = std::chrono::system_clock::to_time_t(now);
    localtime_r(reinterpret_cast<const time_t*>(&inTime), &localtime_);

    return &localtime_;
}

void BaseLogger::endline(LogLevel level, const std::string& message) {
    MutexLockGuard lock(mutex_);
    // 调用纯虚函数，多态，调用派生类具体实现
    output(getLocalTime(), LogLevelInfo.find(level)->second, message);
}

BaseLogger::LogStream::LogStream(BaseLogger& logger, LogLevel level)
    : logger_(logger),
      level_(level) {

}

BaseLogger::LogStream::LogStream(const BaseLogger::LogStream& other)
     //: std::ostringstream(other),
      :logger_(other.logger_),
      level_(other.level_) {

}

BaseLogger::LogStream::~LogStream() {
    logger_.endline(level_, static_cast<std::string>(std::move(str())));
}

// 私有方法，由endline调用，endline加锁保证线程安全
void ConsoleLogger::output(const tm* tmPtr, const std::string& level, const std::string& message) {
    std::cout << '[' << tmPtr << ']'
              << '[' << level << ']'
              << '\t' << message 
              << std::endl;   
}

// 初始化列表基类构造函数
FileLogger::FileLogger(std::string filename) noexcept : BaseLogger() {
    std::string validFilename(filename.size(), '\0');
    std::regex express(R"(/|:| |>|<|\"|\\*|\\?|\\|)");
    std::regex_replace(validFilename.begin(), filename.begin(), filename.end(), express,"-");
    file_.open(validFilename, std::fstream::out | std::fstream::app | std::fstream::ate);

    assert(!file_.fail());
}

FileLogger::~FileLogger() {
    file_.flush();
    file_.close();
}


void FileLogger::output(const tm *tmPtr, const std::string &levelStr, const std::string &messageStr) {
    file_ << '[' << tmPtr << ']'
          << '[' << levelStr << ']'
          << '\t' << messageStr << std::endl;
    file_.flush();
}





