#ifndef CHIVE_BASE_CLOG_CHIVELOG_H
#define CHIVE_BASE_CLOG_CHIVELOG_H

#include <pthread.h>
#include <stdio.h>      // rename()/access()
#include <map>
#include <stdarg.h>     // vsnprintf
#include <string.h>     // 字符串函数集
#include <unistd.h>     // access() 
#include <sys/time.h>
#include <sys/stat.h>   
#include <sys/syscall.h>

#define CLOG_DIR  "/data/chive/clog/"
#define CLOG_FILE  "/data/chive/clog/chive_log_0.txt"
#define CLOG_FILE_TMP  "/data/chive/clog/chive_log_tmp.txt"

#define MAX_BUFFER_SIZE 1024
#define BUFFER_COUNT    1024
#define MAX_PATH_LENGTH 128
#define MAX_FILE_SIZE (64 * (1 << 20))  // 64M

#define gettid() syscall(__NR_gettid)  

// debug级别
enum  CDebugLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    NONE,
    VERB,
    MAXLV,   /*max level*/
};

// debug模块
enum class DebugGroup {
    NONE_GROUP,
    CHIVE_GROUP
};

//日志上下文
struct CLogContext {
    bool    logEnabled;                         //
    FILE*   pLogFile;                           //
    char   filePath[MAX_PATH_LENGTH];          //
    int     maxFileCnt;                         // 已写的最大文件个数
    int     bufLen;                             // 当前全部buf的长度
    int     writeCnt;                           // 已用的buf个数
    int     bufId;                              // 当前写buf数组的下标, < BUFFER_COUNT
    char    buf[BUFFER_COUNT][MAX_BUFFER_SIZE]; // 预分配缓冲区
    pthread_mutex_t bufMutex;                   // buf 互斥访问
    pthread_mutex_t fileMutex;                  // 
};

// 日志级别信息集合
// 当前主要是保存level string name
struct LevelInfoSet{
    unsigned group_enable;  //
    const char *name;       // level name
};


// 声明外部变量，作为全局实例
extern LevelInfoSet g_logLvInfo[CDebugLevel::MAXLV];

#define CHIVE_LOG(module, level, levelString, fmt, args...) \
    if(g_logLvInfo[level].group_enable ) {                       \
        char format[MAX_BUFFER_SIZE];                                   \
        sprintf(format, "%s %s:%d %s() %s", levelString, __FILE__, __LINE__, __FUNCTION__, fmt); \
        logDebugPrint(  \
            (module),   \
            level,      \
            format,     \
            ##args);    \
    }

// 目前支持5种级别打印日志
#define CHIVE_LOG_ERROR(fmt, args...) \
    CHIVE_LOG("CHIVE", CDebugLevel::ERROR, g_logLvInfo[CDebugLevel::ERROR].name, fmt, ##args)
#define CHIVE_LOG_WARN(group, fmt, args...) \
    CHIVE_LOG("CHIVE", CDebugLevel::WARN, g_logLvInfo[CDebugLevel::WARN].name, fmt, ##args)
#define CHIVE_LOG_INFO(group, fmt, args...) \
    CHIVE_LOG("CHIVE", CDebugLevel::INFO, g_logLvInfo[CDebugLevel::INFO].name, fmt, ##args)
#define CHIVE_LOG_DEBUG(group, fmt, args...) \
    CHIVE_LOG("CHIVE", CDebugLevel::DEBUG, g_logLvInfo[CDebugLevel::DEBUG].name, fmt, ##args)
#define CHIVE_LOG_VERB(group, fmt, args...) \
    CHIVE_LOG("CHIVE", CDebugLevel::VERB, g_logLvInfo[CDebugLevel::VERB].name, fmt, ##args)


bool createLogDir(const char *path);

/**
 * 日志打印核心函数
 */
// void logDebugPrint(const char* module, VDebugLevel level, const char* pFormat, ...);
void logDebugPrint(char const * module, CDebugLevel level, char const * pFormat, ...);

/**
 * 将枚举变量名转换为字符串
 * @param group
 * @return 
 */
const char* groupToStr(DebugGroup group);

/**
 * 将字符串转为带时间和线程信息的日志字符串
 * @param strBuffer 未添加时间和线程信息的字符串
 * @return 添加信息后的字符串长度
 */
int strToLog(char* strBuffer);

/**
 * 唤醒线程执行重命名，打开新的log文件
 * @param pLogContext 日志上下文
 */
bool getDebugLogFile(CLogContext* pLogContext);

/**
 * 开始log线程
 */
void startLogPrint(unsigned *groupEnabled);

#endif