/// a test of opening the same file for reading and writing simultaneously

#include <unistd.h>
#include <ctime>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>

int main()
{
    const char* path = "tmp.tmp";
    int fdo = open( path, O_WRONLY | O_CREAT, 0777 );
    int fdi = open( path, O_RDONLY );

    char rbuf[100];
    char wbuf[100];
    size_t rpos = 0;
    timeval start;
    gettimeofday( &start, 0);
    unsigned nread = 0, nwrite = 0;
    while ( true ) {
        timeval tv;
        gettimeofday( &tv, 0 );
        const bool finish = ( tv.tv_sec > start.tv_sec + 20 );
        if ( finish || tv.tv_usec & 1 ) {
            char* buf = rbuf + rpos;
            ssize_t n = read( fdi, buf, sizeof(rbuf)-rpos );
            if ( n == -1 ) {
                printf("read failure: %d\n",n);
            } else if ( n == 0 ) {
                if ( finish ) {
                    printf("read eof\n");
                    break;
                }
                printf("read unavail\n");
            } else {
                printf("read %u bytes\n", unsigned(n));
                char* np;
                while ( (np = static_cast<char*>(memchr(buf,'\n',n))) ) {
                    // 012345678
                    // abcdeNghi
                    *np = '\0';
                    ++nread;
                    printf("read #%u: <%s>\n",nread,buf);
                    const size_t delta = (np - buf) + 1;
                    n -= delta;
                    buf += delta;
                    if ( n == 0 ) break;
                }
                if ( n > 0 ) {
                    memmove(rbuf,buf,n);
                }
                rpos = n;
            }
        } else {
            snprintf(wbuf,sizeof(wbuf),"%lx:%x\n",long(tv.tv_sec),unsigned(tv.tv_usec));
            const size_t n = write( fdo, wbuf, strlen(wbuf) );
            ++nwrite;
            printf("write #%u: %u bytes: <%s>\n",nwrite,unsigned(n),wbuf);
        }
    }
    close(fdo);
    close(fdi);
    return 0;
}
