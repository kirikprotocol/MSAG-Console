#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "logger/Logger.h"

using namespace smsc::logger;

int main()
{
    Logger::initForTest(Logger::LEVEL_DEBUG);
    Logger* mainlog = Logger::getInstance("main");
    smsc_log_info(mainlog,"started");
    const size_t chunksz = 0x1000;
    const size_t blocksz = chunksz*0x100;
    std::vector< void* > blocks;
    const size_t nblocks = 100;
    blocks.reserve(nblocks*2);
    for ( size_t i = 0; i < nblocks; ++i ) {
        void* p = malloc( blocksz );
        blocks.push_back(p);
        smsc_log_info(mainlog,"block of size %x malloced at %p",unsigned(blocksz),p);
    }

    void* delim = malloc( 55 );
    smsc_log_info(mainlog,"block of size %x (delimiter) at %p",55,delim);

    for ( size_t i = 0; i < nblocks; ++i ) {
        void* p = malloc( blocksz );
        blocks.push_back(p);
        smsc_log_info(mainlog,"block of size %x malloced at %p",unsigned(blocksz),p);
    }

    struct ::rusage usage;
    ::getrusage(RUSAGE_SELF,&usage);
#define SHOWUSAGE(x) printf("usage %s = %lu\n",#x, usage.ru_ ##x)
    SHOWUSAGE(maxrss);
    SHOWUSAGE(ixrss);
    SHOWUSAGE(idrss);
    SHOWUSAGE(isrss);
    SHOWUSAGE(minflt);
    SHOWUSAGE(majflt);
    SHOWUSAGE(nswap);
    SHOWUSAGE(inblock);
    SHOWUSAGE(oublock);
    SHOWUSAGE(msgsnd);
    SHOWUSAGE(msgrcv);
    SHOWUSAGE(nsignals);
    SHOWUSAGE(nvcsw);
    SHOWUSAGE(nivcsw);
#undef SHOWUSAGE(x)

    free(delim);

    for ( std::vector< void* >::reverse_iterator i = blocks.rbegin();
          i != blocks.rend(); ++i ) {
        free(*i);
    }
    smsc_log_info(mainlog,"finished");
    return 0;
}
