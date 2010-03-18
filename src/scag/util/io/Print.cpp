#include "Print.h"
#include "core/buffers/TmpBuf.hpp"

namespace scag {
namespace util {

void Print::print( const char* fmt, ... ) throw () 
{
    va_list args;
    va_start( args, fmt );
    this->printva( fmt, args );
    va_end( args );
}

void PrintLog::printva( const char* fmt, va_list args ) throw ()
{
    log_->logva( lev_, fmt, args );
}

void PrintFile::printva( const char* fmt, va_list args ) throw ()
{
    // should we lock ?
    vfprintf( fp_, fmt, args );
    fprintf( fp_, "\n" );
}


void PrintString::printva( const char* fmt, va_list args ) throw ()
{
    vsnprintf(buf_, sz_, fmt, args );
}


int PrintAString::myvasprintf( char** strp, const char* fmt, va_list ap )
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

void PrintAString::printva( const char* fmt, va_list args ) throw ()
{
    free(buf_); buf_ = 0;
    if ( -1 == 
#ifdef __GNUC__
         vasprintf( &buf_, fmt, args ) 
#else
         myvasprintf( &buf_, fmt, args )
#endif
         ) buf_ = 0;
}


void PrintStdString::printva( const char* fmt, va_list args ) throw ()
{
    smsc::core::buffers::TmpBuf<char,256> buf;
    int res = vsnprintf(buf.get(),buf.getSize(),fmt,args);
    if ( res < 0 ) { return; }
    if ( size_t(res) > buf.getSize() ) {
        buf.setSize(res);
        vsnprintf(buf.get(),buf.getSize(),fmt,args);
    }
    str_.append(buf.get());
}

}
}
