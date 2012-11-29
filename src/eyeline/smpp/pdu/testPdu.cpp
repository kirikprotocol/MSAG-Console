#include <time.h>
#include <algorithm>
#include "logger/Logger.h"
#include "util/signal.hpp"
#include "PduInputBuf.h"
#include "testPdu.h"
//#include "testPdu1.h"
#include "testPdu2.h"

using namespace eyeline::smpp;
using namespace smsc::core::synchronization;

extern "C" void sigHandler(int signo)
{
    fprintf(stderr,"signal received %d\n",signo);
//    MutexGuard mg(stopmon);
//    stopping = true;
//    stopmon.notify();
}

smsc::logger::Logger* mainlog;

void registerSignalHandler()
{
    sigset_t st;
    struct sigaction sa;

    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigfillset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, NULL);

    sigfillset(&st);
    sigdelset(&st, SIGBUS);
    sigdelset(&st, SIGFPE);
    sigdelset(&st, SIGILL);
    sigdelset(&st, SIGSEGV);
    sigdelset(&st, SIGINT);
    sigdelset(&st, SIGHUP);  
    sigprocmask(SIG_SETMASK, &st, NULL);

    sigdelset(&st, SIGUSR2);
    sigdelset(&st, SIGALRM);
    sigdelset(&st, SIGABRT);
    sigdelset(&st, SIGTERM);

    sigset(SIGUSR2, sigHandler);
    sigset(SIGFPE,  sigHandler);
    sigset(SIGILL,  sigHandler);
    signal(SIGABRT, sigHandler);
    sigset(SIGALRM, sigHandler);
    sigset(SIGTERM, sigHandler);
    sigset(SIGINT,  sigHandler);
}


int main( int argc, const char** argv )
{
    smsc::logger::Logger::Init();
    mainlog = smsc::logger::Logger::getInstance("mainlog");
    smsc_log_info(mainlog,"main started");
    printf("pause. Debug..."); fflush(stdout);
    getchar();
    printf("\ncontinue\n"); fflush(stdout);

#ifdef SMPPPDUUSEMEMORYPOOL
    PduBuffer::pool.setLogger(smsc::logger::Logger::getInstance("pdu.mem"));
#endif

    registerSignalHandler();

    const char* buf = "str456789_123456789@123456789#123456789$123456789%123456789^123456789&123456789*";
    const uint32_t value = 0x12345678; //90abcdef;

//    test1(value, buf, mainlog);     // test use DeliverSm
    test2(value, buf, mainlog);     // test use GenericNack

    smsc_log_info(mainlog,"main finished");
    // PduBuffer::pool.setLogger(0);
    return 0;
}
