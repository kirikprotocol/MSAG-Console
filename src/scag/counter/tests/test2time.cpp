#include <sys/time.h>
#include <ctime>
#include <string>
#include <iostream>
#include "util/int.h"

int64_t clockfun();
int64_t gettimeofdayfun();
int64_t timefun();

typedef int64_t (*timefun_type)(void);

int main( int argc, char** argv )
{
    const unsigned passes = 1000;
    const unsigned innerpasses = 100000;
    
    if ( argc <= 1 ) {
        std::cout << "usage: prog (-t | -d | -c)" << std::endl;
        return -1;
    }

    std::string arg(argv[1]);
    timefun_type fun = 0;
    if ( arg == "-t" ) {
        fun = timefun;
    } else if ( arg == "-c" ) {
        fun = clockfun;
    } else if ( arg == "-d" ) {
        fun = gettimeofdayfun;
    } else {
        std::cout << "see usage" << std::endl;
        return -1;
    }

    for ( size_t k = 0; k < passes; ) {
        int64_t v = 0;
        for ( size_t i = 0; i < innerpasses; ++i ) {
            v += (*fun)() / 1000;
        }
        ++k;
        if ( (k % 20) == 0 ) {
            std::cout << "k=" << k << " v=" << (v / innerpasses) << std::endl;
        }
    }
    return 0;
}

int64_t clockfun()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME,&ts);
    return int64_t(ts.tv_sec)*1000000 + ts.tv_nsec/1000;
}

int64_t gettimeofdayfun()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return int64_t(tv.tv_sec)*1000000 + tv.tv_usec;
}

int64_t timefun()
{
    time_t tt = time(NULL);
    return int64_t(tt)*1000000;
}
