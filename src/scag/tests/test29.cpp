/// a test of time conversion
#include <time.h>
#include <iostream>

std::string fmtTime( struct tm* t )
{
    char buf[30];
    sprintf(buf,"%04u-%02u-%02u+%02u:%02u:%02u",
            t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec );
    return buf;
}


time_t timediff;

time_t mktimegm( struct tm* t )
{
    return mktime(t) + timediff;
}


int main()
{
    const time_t now = time(0);
    std::cout << "now is " << now << std::endl;
    struct tm gmnow;
    std::cout << "gmtime(now) = " << fmtTime(gmtime_r(&now,&gmnow)) << std::endl;
    struct tm locnow;
    std::cout << "localtime(now) = " << fmtTime(localtime_r(&now,&locnow)) << std::endl;

    std::cout << "mktime(gmnow) = " << mktime(&gmnow) << std::endl;
    std::cout << "mktime(locnow) = " << mktime(&locnow) << std::endl;

    timediff = mktime(&locnow) - mktime(&gmnow);
    std::cout << "timediff = " << timediff << std::endl;

    std::cout << "mktimegm(gmnow) = " << mktimegm(&gmnow) << std::endl;
    std::cout << "mktimegm(locnow) = " << mktimegm(&locnow) << std::endl;

    std::cout << "# --- a test of time offset calculation" << std::endl;
    struct tm t0 = {0};
    t0.tm_mday = 1;
    t0.tm_year = 110;
    std::cout << "t0 = " << fmtTime(&t0) << std::endl;
    time_t loc = mktime(&t0);
    time_t gmt = mktime(gmtime_r(&loc,&t0));
    std::cout << "loc = " << loc << std::endl;
    std::cout << "gmt = " << gmt << std::endl;
    std::cout << "diff = " << loc - gmt << std::endl;

    const time_t zero = loc - gmt + loc;
    std::cout << "zero = " << zero << std::endl;
    std::cout << "gmtime(zero) = " << fmtTime(gmtime(&zero)) << std::endl;
    return 0;
}
