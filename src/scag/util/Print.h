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
private:
    smsc::logger::Logger*          log_;
    smsc::logger::Logger::LogLevel lev_;
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
private:
    FILE* fp_;
};


class PrintString : public Print
{
public:
    PrintString( char* buf, size_t sz ) : Print(buf), buf_(buf), sz_(sz) {}

    const char* buf() const {
        return buf_;
    }

protected:
    virtual void printva( const char* fmt, va_list args ) throw () {
        vsnprintf(buf_, sz_, fmt, args );
    }

private:
    char* buf_; // not owned
    size_t sz_;
};


class PrintAString : public Print
{
private:
    /// an analog of vasprintf
    static int myvasprintf( char** strp, const char* fmt, va_list ap )
    {
        assert( strp && fmt );

        // remember the arguments
        va_list aq;
        va_copy( aq, ap );

        // a guess
        size_t sz = strlen(fmt) * 3;
        *strp = reinterpret_cast< char* >( malloc(sz) );
        // printf( "pre-allocation: %u\n", sz );
        int res = vsnprintf( *strp, sz, fmt, ap );
        // printf( "vsnprintf: %d\n", res );
        if ( res < 0 ) {
            free( *strp );
            *strp = 0;
            return res;
        }
        if ( size_t(res) > sz ) {
            // too small buffer
            free( *strp );
            sz = size_t(res);
            // printf( "re-allocation: %u\n", sz);
            *strp = reinterpret_cast< char* >( malloc(sz) );
            int bsz = vsnprintf( *strp, sz, fmt, aq );
            assert( sz == size_t(bsz) );
        }
        va_end( aq );
        return res;
    }

public:
    virtual ~PrintAString() throw () { free(buf_); }
    PrintAString() : Print(true), buf_(0) {}
    
    virtual void printva( const char* fmt, va_list args ) throw () {
        free(buf_); buf_ = 0;
        if ( -1 == 
#ifdef _GNU_SOURCE
             vasprintf( &buf_, fmt, args ) 
#else
             myvasprintf( &buf_, fmt, args )
#endif
             ) buf_ = 0;
    }

    const char* buf() const {
        return buf_;
    }

private:
    PrintAString( const PrintAString& );
    PrintAString& operator = ( const PrintAString& );
    
private:
    char* buf_;
};

}
}

namespace scag2 {
namespace util {
using scag::util::Print;
}
}

#endif /* ! _SCAG_UTIL_PRINT_H */
