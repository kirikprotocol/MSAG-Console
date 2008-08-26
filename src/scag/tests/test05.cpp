#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/// an analog of vasprintf
int myvasprintf( char** strp, const char* fmt, va_list ap )
{
    assert( strp && fmt );

    // remember the arguments
    va_list aq;
    va_copy( aq, ap );

    // a guess
    size_t sz = strlen(fmt) * 3;
    *strp = reinterpret_cast< char* >( malloc(sz) );
    printf( "pre-allocation: %u\n", sz );
    int res = vsnprintf( *strp, sz, fmt, ap );
    printf( "vsnprintf: %d\n", res );
    if ( res < 0 ) {
        free( *strp );
        *strp = 0;
        return res;
    }
    if ( size_t(res) > sz ) {
        // too small buffer
        free( *strp );
        sz = size_t(res);
        printf( "re-allocation: %u\n", sz);
        *strp = reinterpret_cast< char* >( malloc(sz) );
        int bsz = vsnprintf( *strp, sz, fmt, aq );
        assert( sz == size_t(bsz) );
    }
    va_end( aq );
    return res;
}


int myasprintf( char** strp, const char* fmt, ... )
{
    va_list ap;
    va_start(ap, fmt);
    int res = myvasprintf( strp, fmt, ap );
    va_end(ap);
    return res;
}


#ifdef __cplusplus
}
#endif

int main()
{
    char* buf;
    int times = 3;
    if ( -1 == myasprintf( &buf, "hello, %s.  Should I say this %d times?",
                           "world", times ) ) buf = 0;
    printf( "buf='%s'\n", buf ? buf : "" );
    free(buf);

    if ( -1 == myasprintf( &buf, "%d%u%x", 1111111, 222222, 3333333 ) ) buf = 0;
    printf( "buf='%s'\n", buf ? buf : "" );
    free(buf);

    return 0;
}
