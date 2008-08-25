#ifndef _SCAG_UTIL_PRINT_H
#define _SCAG_UTIL_PRINT_H

#include <cstdio>
#include <cstdarg>
#include "logger/Logger.h"

#define scag_print(x, fmt, ...) if (x.enabled()) x.print(fmt,__VA_ARGS__);
#define scag_log(name,x,lev)  scag::util::PrintLog(x,lev) name
#define scag_log_debug(name,x) scag_log(name,x,smsc::logger::Logger::LEVEL_DEBUG)
#define scag_log_info(name,x) scag_log(name,x,smsc::logger::Logger::LEVEL_INFO)
#define scag_log_warn(name,x) scag_log(name,x,smsc::logger::Logger::LEVEL_WARN)
#define scag_log_error(name,x) scag_log(name,x,smsc::logger::Logger::LEVEL_ERROR)

// unified print object
namespace scag {
namespace util {

class Print
{
protected:
    explicit Print( bool en = true ) : enabled_(en) {}
public:
    virtual ~Print() throw () {}
    inline bool enabled() const throw () { return enabled_; }
    inline void print( const char* fmt, ... ) throw () {
        va_list args;
        va_start( args, fmt );
        this->printva( fmt, args );
        va_end( args );
    }
protected:
    virtual void printva( const char* fmt, va_list args ) throw () = 0;
protected:
    bool enabled_;
};


class PrintLog : public Print
{
public:
    PrintLog( smsc::logger::Logger*          log,
              smsc::logger::Logger::LogLevel lev ) : 
    Print( log && log->isLogLevelEnabled(lev) ), log_(log), lev_(lev) {}

protected:
    virtual void printva( const char* fmt, va_list args ) throw () {
        log_->logva( lev_, fmt, args );
    }
};


class PrintFile : public Print
{
public:
    PrintFile( FILE* fp ) : Print( fp ), fp_(fp) {}

protected:
    virtual void printva( const char* fmt, va_list args ) throw () {
        // should we lock ?
        vfprintf( fp_, fmt, args );
        fprintf( fp_, "\n" );
    }
};

}
}

namespace scag2 {
namespace util {
using scag::util::Print;
}
}

#endif /* ! _SCAG_UTIL_PRINT_H */
