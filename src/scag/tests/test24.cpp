#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <exception>
#include <algorithm>
#include <ctime>

#include "util/int.h"

/// POC: infosme with a log-file

void writeLog( int fd,
               uint8_t  state,
               uint32_t msgId,
               uint32_t regionId,
               uint32_t textId,
               uint64_t msgpos,
               const char* userdata,
               const char* receipt )
{
    char buf[200];
    int n = std::sprintf(buf,"%u,%u,%u,%u,%s,%llx,%s\n",state,msgId,regionId,textId,userdata,msgpos,receipt);
    if ( write(fd,buf,n) < 0 ) {
        std::fprintf(stderr,"write failed: %d\n",errno);
        std::terminate();
    }
}


void readLog( const char* buf, size_t buflen )
{
    unsigned state;
    unsigned msgId;
    unsigned regionId;
    unsigned textId;
    uint64_t msgpos;
    char userdata[64];
    char receipt[128];
    std::sscanf(buf,"%u,%u,%u,%u,%[^,]s,%llx,%s",&state,&msgId,&regionId,&textId,userdata,&msgpos,receipt);
}


void usage( const char* prog )
{
    std::fprintf( stderr, "usage: %s (r|w) FILENAME\n",prog);
    std::terminate();
}


int main( int argc, char** argv )
{
    uint32_t nrecords = 10000000;
    const uint32_t messagesPerFile = 100000;

    if ( argc < 3 ) {
        usage(argv[0]);
    }

    bool reading;
    if ( std::strchr(argv[1],'r') ) {
        // reading
        reading = true;
    } else if ( std::strchr(argv[1],'w') ) {
        reading = false;
    } else {
        usage(argv[0]);
    }
#if defined(sun)
    bool usedirectio;
    if ( std::strchr(argv[1],'d') ) {
        usedirectio = true;
    } else {
        usedirectio = false;
    }
#endif

    const char* fname = argv[2];

    timeval start;
    gettimeofday(&start,0);

    if ( reading ) {
        // reading
        int fd = open(fname,O_RDONLY);
        if ( fd < 0 ) {
            std::fprintf(stderr,"cannot open %s for reading: %d\n",fname,fd);
            std::terminate();
        }
#if defined(sun)        
        if (usedirectio) {
            directio(fd,DIRECTIO_ON);
        }
#endif
        nrecords = 0;
        const size_t prefsize = 8192;
        const size_t extrasize = 256;
        char rbuf[prefsize+extrasize];
        size_t rpos = 0;
        while (true) {
            char* buf = rbuf+rpos;
            ssize_t n = read(fd,buf,std::min(prefsize,sizeof(rbuf)-rpos));
            if ( n == -1 ) {
                std::fprintf(stderr,"read failed: %d\n",errno);
                break;
            } else if ( n == 0 ) {
                std::fprintf(stderr,"eof\n");
                break;
            }
            char* np;
            while ( (np = static_cast<char*>(std::memchr(buf,'\n',n))) ) {
                *np = '\0';
                size_t delta = np - buf;
                if ( nrecords ) {
                    // parse the line
                    readLog(buf,delta);
                }
                ++delta;
                n -= delta;
                buf += delta;
                ++nrecords;
                if ( n == 0 ) break;
            }
            if (n>0) {
                std::memmove(rbuf,buf,n);
            }
            rpos = n;
        }
        close(fd);
    } else {
        // writing
        int fd = open(fname,O_CREAT|O_TRUNC|O_WRONLY,0777);
        if ( fd < 0 ) {
            std::fprintf(stderr,"cannot open %s for writing: %d\n",fname,fd);
            std::terminate();
        }
#if defined(sun)        
        if (usedirectio) {
            directio(fd,DIRECTIO_ON);
        }
#endif
        const char* header = "#state,msgId,regId,textId,userData,msgPos,receipt\n";
        write(fd,header,std::strlen(header));
        for ( uint32_t i = 1; i <= nrecords; ++i ) {
            const uint32_t msgId = i;
            const uint32_t regionId = i % 5 + 1;
            const uint32_t textId = i % 100 + 1;
            const uint64_t msgpos = (i / messagesPerFile + 1) * 0x100000000ULL +
                msgId % messagesPerFile;
            const uint8_t state = i % 3;
            char userdata[30];
            std::sprintf(userdata,"%x%x",unsigned(i^0xcececece),unsigned(i^0xfafafafa));
            char receipt[40];
            if ( state == 2 ) {
                std::sprintf(receipt,"%llx%llx",msgpos^0xfefefefefefefefeULL,i^0xababababababababULL);
            } else {
                receipt[0] = '\0';
            }
            writeLog( fd, state, msgId, regionId, textId, msgpos, userdata, receipt );
        }
        fsync(fd);
        close(fd);
    }

    timeval finish;
    gettimeofday(&finish,0);
    const uint64_t runtime =
        (finish.tv_sec*1000ULL+finish.tv_usec/1000) -
        (start.tv_sec*1000ULL+start.tv_usec/1000);
    std::printf("total running time of %s %u records: %llu msec\n",
                reading ? "reading" : "writing", unsigned(nrecords), runtime);
}
