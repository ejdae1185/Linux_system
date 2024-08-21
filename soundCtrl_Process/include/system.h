#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#define __USE_POSIX199309
#include "type.h"
#include <time.h>
#include <stdarg.h>
#include <stdio.h>

#define BHGRN "\e[1;32m"
#define CRESET "\e[0m"

// 로그 타입 정의
#define LOG_NORMAL 0

// 파일 이름 매크로
#ifndef __FILENAME__
#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
#define BHPRP "\033[1;35m" // 보라색

// RFC8601 시간 형식 함수 (인라인)
static inline void rfc8601_timespec(char *buf, struct timespec *ts)
{
    struct tm tm;
    gmtime_r(&ts->tv_sec, &tm);
    snprintf(buf, 256, "%04d-%02d-%02dT%02d:%02d:%02dZ",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);
}

// 디버그 버퍼에 삽입하는 함수 (인라인)
static inline void insertDebugBuf(int logType, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// _prn 매크로 정의
#define _prn(tag, color, logType, fmt, ...)                                   \
    do                                                                        \
    {                                                                         \
        struct timespec cur;                                                  \
        clock_gettime(CLOCK_REALTIME, &cur);                                  \
        char rfc8601Time[256];                                                \
        rfc8601_timespec(rfc8601Time, &cur);                                  \
        insertDebugBuf(logType,                                               \
                       (char *)color "[%s] %s:%s, %s():%d: " fmt CRESET "\n", \
                       rfc8601Time, tag, __FILENAME__, __func__,              \
                       __LINE__ __VA_OPT__(, ) __VA_ARGS__);                  \
    } while (0)

// PrintDbg 매크로 정의
#define PrintDbg(fmt, ...) _prn("[DEBG]", BHGRN, LOG_NORMAL, fmt, __VA_ARGS__)
#define PrintErr(fmt, ...) _prn("[ERRO]", BHPRP, LOG_NORMAL, fmt, __VA_ARGS__)

#endif // __SYSTEM_H__