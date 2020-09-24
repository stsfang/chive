#include "chiveLog.h"
#include <string>

//外部变量定义
LevelInfoSet g_logLvInfo[CDebugLevel::MAXLV] = {
    {1, "[DEBUG]"},
    {1, " [INFO]"},
    {1, " [WARN]"},
    {1, "[ERROR]"},
    {1, " [NONE]"},
    {1, " [VERB]"},
};

// 静态全局日志上下文
/** 
 * pthread.h 静态初始化
 * #define PTHREAD_MUTEX_INITIALIZER (pthread_mutex_t)GENERIC_INITIALIZER
 * #define GENERIC_INITIALIZER				((void *) (size_t) -1)
 */
static CLogContext logContext = {
    false,
    NULL,
    "/data/chive/clog/chive_log_0.txt",
    0,
    0,
    0,
    0,
    {"",""},
    PTHREAD_MUTEX_INITIALIZER,      
    PTHREAD_MUTEX_INITIALIZER
};

static pthread_t fileThread;
static int flagThread = -1;
static pthread_cond_t newfileCondition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t newfileMutex = PTHREAD_MUTEX_INITIALIZER;


void writeToConsole(const char* logLine) {
    printf("%s\n", logLine);
}

const char* getFileName(const char* filepath) {
    // C 库函数 char *strrchr(const char *str, int c) 
    // 在参数 str 所指向的字符串中搜索最后一次出现字符 c（一个无符号字符）的位置
    const char* filename = strrchr(filepath, static_cast<int>('/'));
    if( NULL != filename) {
        filename += 1;  // 后移一个位置才是文件名开头
    } else {
        filename = filepath;
    }
    return filename;
}

void logDebugPrint(char const * module, CDebugLevel level, char const * pFormat, ...) {
    // 没有开启log，直接返回
    if(logContext.logEnabled  == 0) {
        return;
    }

    char strBuffer[MAX_BUFFER_SIZE];

    // 输出日志字符串
    va_list args;   //获取 ... 参数列表
    va_start(args, pFormat);
    vsnprintf(strBuffer, sizeof(strBuffer), pFormat, args);
    va_end(args);

    // 写出到控制台
    // if ((level == CDebugLevel::ERROR)
    //         || (level == CDebugLevel::DEBUG)
    //         || (strcmp(module, "CHIVE") == 0 && level == CDebugLevel::INFO) ) {
    //     /// FIXME:
    //     writeToConsole(strBuffer);
    // }

    // file未创建
    if (logContext.pLogFile == NULL) {
        return;
    }
    // 加上时间和线程等信息
    int len = strToLog(strBuffer);
    // strBuffer ends with '\0', so console log based on 'printf'
    // can perform normally
    writeToConsole(strBuffer);
    // since '\0' means the end-of-line, so replace '\0' with '\n'
    // to make console log compatible with file log
    if (strBuffer[len-1] == '\0' ) {
        strBuffer[len-1] = '\n';
    } else if(strBuffer[len-1] != '\n') {
        strBuffer[len++] = '\n';
    }

    pthread_mutex_lock(&(logContext.bufMutex));
    if (logContext.bufLen + len < MAX_BUFFER_SIZE) {
        // 当前buf bucket 偏移 bufLen开始写入长度为len的strBuffer
        memcpy(logContext.buf[logContext.bufId] + logContext.bufLen, strBuffer, len);
        logContext.bufLen += len;   // 更新当前buf bucket的长度
        pthread_mutex_unlock(&(logContext.bufMutex));
    } 
    else {      // 当前buf bucket不够写，需要换到下一个buf bucket
        int curBufId = logContext.bufId;
        int curBufLen = logContext.bufLen;

        // 准备一个新的buf bucket
        logContext.bufId = (logContext.bufId + 1) % BUFFER_COUNT;
        memset(logContext.buf[logContext.bufId], 0, MAX_BUFFER_SIZE);   //清空buf
        memcpy(logContext.buf[logContext.bufId], strBuffer, len);
        logContext.bufLen = len;    //更新new buf bucket 的长度

        // 如果当前缓冲区的日志已经超过一个文件的大小，需要换到新文件
        // 换新文件的逻辑是对旧文件重命名，然后打开新文件
        if ((logContext.writeCnt + 1) * MAX_BUFFER_SIZE > MAX_FILE_SIZE) {
            getDebugLogFile(&logContext);   // 通知线程重命名文件
        }
        pthread_mutex_unlock(&logContext.bufMutex);

        // 写出一个已写满的buf到文件
        pthread_mutex_lock(&logContext.fileMutex);
        if (NULL != logContext.pLogFile) {
            ++logContext.writeCnt;
            ///FIXME: 这里为什么需要一个>=的判断
            if (curBufLen >= MAX_BUFFER_SIZE) {     
                logContext.buf[curBufId][MAX_BUFFER_SIZE-1] = '\0';
            }
            else {
                logContext.buf[curBufId][curBufLen] = '\0';
            }
            fprintf(logContext.pLogFile, "%s", logContext.buf[curBufId]);
        }
        pthread_mutex_unlock(&(logContext.fileMutex));
    }
}

int strToLog(char* strBuffer) {
    char newStrBuf[MAX_BUFFER_SIZE];
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    struct tm* curTime = localtime((time_t*)&tv.tv_sec);

    if (curTime != NULL) {
        // 格式 年-月-日 时:分:秒:微秒 进程号 线程号 格式化字符串
        snprintf(newStrBuf, MAX_BUFFER_SIZE - 1, "%04d-%02d-%02d %02d:%02d:%02d.%03ld %4d %4ld %s",
                    curTime->tm_year + 1900, curTime->tm_mon+1, curTime->tm_mday, 
                    curTime->tm_hour, curTime->tm_min, curTime->tm_sec, tv.tv_usec / 1000,
                    getpid(), gettid(),
                    strBuffer);
    }
    int len = strlen(newStrBuf);
    /// fix bug#memcpy vs strcpy
    /// memcpy 拷贝前num个,忽略\0; strcpy 遇到 \0 结束拷贝
    /// 所以这里必须是len+1才能把 \0 也拷贝过去
    memcpy(strBuffer, newStrBuf, len+1);
    // strBuffer[len] = '\0';
    return len+1;
}

bool getDebugLogFile(CLogContext* pLogContext) {
    pLogContext->writeCnt = 0;
    if (pLogContext->pLogFile != NULL) {
        fclose(pLogContext->pLogFile);
    }

    pthread_mutex_lock(&newfileMutex);
    if (access(CLOG_FILE, F_OK) == 0) {  // 检查CLOG_FILE是否存在
        // 将 chive_log_0.txt 重命名为 chive_log_tmp.txt
        // 然后通知后台线程对CLOG_DIR目录下的log文件按序重命名
        if (rename(CLOG_FILE, CLOG_FILE_TMP) == -1) {   // 对CLOG_FILE重命名
            pthread_mutex_unlock(&newfileMutex);        // 重名名失败,释放锁，返回false
            return false;
        }
    }
    // 唤醒重命名线程
    pthread_cond_signal(&newfileCondition);
    pthread_mutex_unlock(&newfileMutex);

    // 完成重命名之后，打开新文件 chive_log_0.txt，新的log继续写入到chive_log_0.txt
    if((pLogContext->pLogFile = fopen(pLogContext->filePath, "a")) == NULL) {
        return false;
    }
    setbuf(pLogContext->pLogFile, NULL);    // 设置文件流的缓冲区为Null即不用缓冲区
    return true;
}

void* fileThreadProcess(void* arg) {
    printf("rename thread on...\n");
    while(true) {
        pthread_mutex_lock(&newfileMutex);
        pthread_cond_wait(&newfileCondition, &newfileMutex);

        char fileSrc[MAX_PATH_LENGTH], fileDst[MAX_PATH_LENGTH];
        // VLOG_DIR目录下的log文件重新按序命名
        // e.g.  chive_log_11.txt ==> chive_log_12.txt
        // ...
        // chive_log_1.txt ==> chive_log_2.txt
        for (int i = logContext.maxFileCnt - 2; i > 0; --i) {
            sprintf(fileSrc, "%s%s%d%s", CLOG_DIR, "chive_log_", i, ".txt");
            if (access(fileSrc, F_OK) == 0) {
                sprintf(fileDst, "%s%s%d%s", CLOG_DIR, "chive_log_", i+1, ".txt");
                if(rename(fileSrc, fileDst) == -1) {
                    break;
                }
            }
        }

        // chive_log_tmp.txt ==> chive_log_1.txt
        // 如此就能保证每次把最新log写入到文件 chive_log_0.txt
        sprintf(fileSrc, "%s%s%s", CLOG_DIR, "chive_log_tmp", ".txt");
        sprintf(fileDst, "%s%s%s", CLOG_DIR, "chive_log_1", ".txt");
        if(access(fileSrc, F_OK) == 0) {
            rename(fileSrc, fileDst);
        }
        pthread_mutex_unlock(&newfileMutex);
    }
}

void startLogPrint(unsigned *groupEnabled) {
    if(-1 == flagThread) {
        flagThread = pthread_create(&fileThread, NULL, fileThreadProcess, NULL);
        if(flagThread) {
            printf("create thread failed\n");
        }
    }
    // enable log
    logContext.logEnabled = true;

    if (access(CLOG_DIR, 0) != 0 && !createLogDir(CLOG_DIR)) {
        printf("create dir failed\n");
        return;
    }
    logContext.maxFileCnt = 128;
    // 创建log文件填充pLogFile
    if (NULL == logContext.pLogFile && !getDebugLogFile(&logContext)) {
        printf("get debug file failed\n");
        return;
    }
    // 初始化开启的log level
    ///FIXME:

    if (NULL != logContext.pLogFile && access(logContext.filePath, F_OK) != 0) {
        getDebugLogFile(&logContext);
    }
}

bool createLogDir(const char *path) {
    printf("create log dir\n");
    std::string builder;
    std::string sub;
    std::string folder(path);

    for(auto it = folder.begin(); it != folder.end(); ++it) {
        const char c = *it;
        sub.push_back(c);
        if( c == '/' || it == folder.end() - 1) {
            builder.append(sub);
            printf("%s\n", builder.c_str());
            if (0 != access(builder.c_str(), 0)) {  // 检查子路径是否存在，不存在就创建
                if (-1 == mkdir(builder.c_str(), 0777)) {
                    return false;
                }
            }
            sub.clear();
        }
    }
    return true;
}
