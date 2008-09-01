//
// a test of iconv conversion
//
#include <cstdio>
#include <cassert>
#include <cstring>
#include <string>
#include <iconv.h>
#include <cerrno>

void dump( const char* s, size_t sz )
{
    unsigned cnt = 0;
    const char* p = s;
    for ( ; sz-- > 0 ; ++p ) {
        printf( " %#2.2x", unsigned(*p) );
        if ( ++cnt >= 8 ) {
            printf( "\n" );
            cnt = 0;
        }
    }
    if ( cnt ) printf( "\n" );
}


std::string cvt( const std::string& from, const std::string& to,
                 const std::string& inbuf )
{
    printf( "convertion from: %s to %s\n", from.c_str(), to.c_str() );

    // get iconv
    iconv_t cd = iconv_open( to.c_str(), from.c_str() );
    if ( cd == iconv_t(-1) ) {
        perror("iconv_open");
        exit(-1);
    }
    
    char result[2000];

    size_t inpos = inbuf.size();
    size_t outpos = sizeof(result);
    char* in = const_cast<char*>(inbuf.data());
    char* out = result;
    size_t left = iconv( cd, &in, &inpos, &out, &outpos );

    printf("left=%u errno=%d\n", left, errno );
    dump( result, sizeof(result)-outpos );
    std::string ret;
    ret.append( result, sizeof(result)-outpos );

    iconv_close( cd );
    return ret;
}


int main( int argc, char** argv )
{
    if ( argc < 4 ) return -1;
    const std::string from(argv[1]);
    const std::string to(argv[2]);
    std::string msg(argv[3]);
    const char* p = "";
    msg.append( p, 1 );

    printf("before:\n");
    dump( msg.data(), msg.size() );

    std::string got = cvt( from, to, msg );

    printf("after:\n");
    dump( got.data(), got.size() );

    std::string back = cvt( to, from, got );

    printf("back:\n");
    dump( back.data(), back.size() );

    assert( msg == back );
    return 0;
}
