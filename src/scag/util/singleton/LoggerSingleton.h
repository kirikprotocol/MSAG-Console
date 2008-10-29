#ifndef _SCAG_UTIL_SINGLETON_LOGGERSINGLETON_H
#define _SCAG_UTIL_SINGLETON_LOGGERSINGLETON_H

#include "Singleton2.h"

namespace scag {
namespace util {
namespace singleton {

class LoggerSingletonImpl;

/// NOTE: this class is to provide proper and guaranteed order of logger destruction
class LoggerSingleton
{
public:
    static LoggerSingleton& Instance();
    LoggerSingleton();
    ~LoggerSingleton();
private:
    LoggerSingleton( const LoggerSingleton& );
};

}
}
}

#endif /* !_SCAG_UTIL_SINGLETON_LOGGERSINGLETON_H */
