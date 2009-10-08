#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include "Time.h"

int main( int argc, char** argv )
{
    char** p = argv;
    for ( ++p; *p != 0; ++p ) {
        char* endptr;
        scag2::util::msectime_type tv = strtoll(*p,&endptr,0);
        if ( *endptr != '\0' ) {
            ::printf("%s is not a number\n",*p);
            continue;
        }
        time_t seconds = time_t(tv / 1000);
        struct tm* locTime = localtime(&seconds);
        if ( ! locTime ) {
            ::printf("cannot convert seconds %llu to struct tm\n",
                   static_cast<long long>(seconds));
            continue;
        }
        ::printf("%s = %04u-%02u-%02u+%02u:%02u:%02u,%03u\n",*p,
               locTime->tm_year + 1900,
               locTime->tm_mon + 1,
               locTime->tm_mday,
               locTime->tm_hour,
               locTime->tm_min,
               locTime->tm_sec,
               unsigned(tv % 1000) );
    }
    return 0;
}
