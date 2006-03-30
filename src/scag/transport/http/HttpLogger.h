#ifndef SCAG_TRANSPORT_HTTP_LOGGER
#define SCAG_TRANSPORT_HTTP_LOGGER

#include "logger/Logger.h"

#define SCAG_HTTP

#if 1

#ifndef _WIN32
#define http_log_fatal(...) if (!httpLogger->isFatalEnabled()) ;else httpLogger->log_(smsc::logger::Logger::LEVEL_FATAL, __VA_ARGS__)
#define http_log_error(...) if (!httpLogger->isErrorEnabled()) ;else httpLogger->log_(smsc::logger::Logger::LEVEL_ERROR, __VA_ARGS__)
#define http_log_warn(...) if (!httpLogger->isWarnEnabled()) ;else httpLogger->log_(smsc::logger::Logger::LEVEL_WARN, __VA_ARGS__)
#define http_log_info(...) if (!httpLogger->isInfoEnabled()) ;else httpLogger->log_(smsc::logger::Logger::LEVEL_INFO, __VA_ARGS__)
#define http_log_debug(...) if (!httpLogger->isDebugEnabled()) ;else httpLogger->log_(smsc::logger::Logger::LEVEL_DEBUG, __VA_ARGS__)
#endif // _WIN32

#else 
#include <stdio.h>

#define http_log_fatal(...) {if (httpLogger->isFatalEnabled()){\
    printf(__VA_ARGS__);printf("\n");fflush(stdout);\
    httpLogger->log_(smsc::logger::Logger::LEVEL_FATAL, __VA_ARGS__);}}
#define http_log_error(...) {if (httpLogger->isErrorEnabled()){\
    printf(__VA_ARGS__);printf("\n");fflush(stdout);\
    httpLogger->log_(smsc::logger::Logger::LEVEL_ERROR, __VA_ARGS__);}}
#define http_log_warn(...) {if (httpLogger->isWarnEnabled()){\
    printf(__VA_ARGS__);printf("\n");fflush(stdout);\
    httpLogger->log_(smsc::logger::Logger::LEVEL_WARN, __VA_ARGS__);}}
#define http_log_info(...) {if (httpLogger->isInfoEnabled()){\
    printf(__VA_ARGS__);printf("\n");fflush(stdout);\
    httpLogger->log_(smsc::logger::Logger::LEVEL_INFO, __VA_ARGS__);}}
#define http_log_debug(...) {if (httpLogger->isDebugEnabled()){\
    printf(__VA_ARGS__);printf("\n");fflush(stdout);\
    httpLogger->log_(smsc::logger::Logger::LEVEL_DEBUG, __VA_ARGS__);}}

#endif

using namespace smsc::logger;

namespace scag { namespace transport { namespace http
{
extern Logger *httpLogger;

}}}

#endif // SCAG_TRANSPORT_HTTP_LOGGER
