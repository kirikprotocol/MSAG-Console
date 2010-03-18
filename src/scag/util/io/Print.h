#ifndef _SCAG_UTIL_PRINT_H
#define _SCAG_UTIL_PRINT_H

#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cassert>
#include <cstring>

#include "logger/Logger.h"

#define scag_print(x,fmt,...) if (x.enabled()) x.print(fmt,__VA_ARGS__);

#define scag_plog(name,x,lev)  scag::util::PrintLog name(x,lev)
#define scag_plog_debug(name,x) scag_plog(name,x,smsc::logger::Logger::LEVEL_DEBUG)
#define scag_plog_info(name,x) scag_plog(name,x,smsc::logger::Logger::LEVEL_INFO)
#define scag_plog_warn(name,x) scag_plog(name,x,smsc::logger::Logger::LEVEL_WARN)
#define scag_plog_error(name,x) scag_plog(name,x,smsc::logger::Logger::LEVEL_ERROR)

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
    void print( const char* fmt, ... ) throw ();
    virtual void printva( const char* fmt, va_list args ) throw () = 0;
protected:
    bool enabled_;
};


inline void fprintf(util::Print* log, const char* format,...) {
    va_list args;
    va_start(args,format);
    log->printva(format,args);
    va_end(args);
}


class PrintLog : public Print
{
public:
    PrintLog( smsc::logger::Logger*          log,
              smsc::logger::Logger::LogLevel lev ) : 
    Print( log && log->isLogLevelEnabled(lev) ), log_(log), lev_(lev) {}
    virtual void printva( const char* fmt, va_list args ) throw ();
private:
    smsc::logger::Logger*          log_;
    smsc::logger::Logger::LogLevel lev_;
};


class PrintFile : public Print
{
public:
    PrintFile( FILE* fp ) : Print( fp ), fp_(fp) {}
    virtual void printva( const char* fmt, va_list args ) throw ();
private:
    FILE* fp_;
};


class PrintString : public Print
{
public:
    PrintString( char* buf, size_t sz ) : Print(buf), buf_(buf), sz_(sz) {}

    inline const char* buf() const {
        return buf_;
    }
    virtual void printva( const char* fmt, va_list args ) throw ();

private:
    char* buf_; // not owned
    size_t sz_;
};


class PrintAString : public Print
{
private:
    /// an analog of vasprintf
    static int myvasprintf( char** strp, const char* fmt, va_list ap );

public:
    virtual ~PrintAString() throw () { free(buf_); }
    PrintAString() : Print(true), buf_(0) {}
    
    virtual void printva( const char* fmt, va_list args ) throw ();

    const char* buf() const {
        return buf_;
    }

private:
    PrintAString( const PrintAString& );
    PrintAString& operator = ( const PrintAString& );
    
private:
    char* buf_;
};


class PrintStdString : public Print
{
public:
    virtual ~PrintStdString() throw() {}
    PrintStdString() : Print(true) {}
    virtual void printva( const char* fmt, va_list args ) throw();
    const char* buf() const {
        return str_.c_str();
    }
    void reset() { str_.clear(); }
private:
    std::string str_;
};

}
}

namespace scag2 {
namespace util {
using scag::util::Print;
using scag::util::PrintLog;
using scag::util::PrintFile;
using scag::util::PrintString;
using scag::util::PrintAString;
using scag::util::PrintStdString;
}
}

#endif /* ! _SCAG_UTIL_PRINT_H */
