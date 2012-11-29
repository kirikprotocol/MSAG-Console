#include <time.h>
#include "logger/Logger.h"
#include "util/signal.hpp"
#include "SmppServer.h"
#include "core/threads/ThreadPool.hpp"
#include "eyeline/smpp/pdu/DeliverSmPdu.h"
#include "eyeline/smpp/pdu/SubmitSmPdu.h"
#include "eyeline/smpp/transport/SmppTestCore.h"
#include "eyeline/smpp/transport/SmppTestTask.h"

using namespace eyeline::smpp;
using namespace smsc::core::synchronization;
using namespace smsc::core::threads;

EventMonitor stopmon;
bool stopping = false;
const char* stopfile = "server.stop";
smsc::logger::Logger* mainlog = 0;

extern "C" void sigHandler(int signo)
{
    fprintf(stderr,"signal received %d\n",signo);
    MutexGuard mg(stopmon);
    stopping = true;
    stopmon.notify();
}


extern "C" void atExitHandler(void)
{
    if (mainlog) {
        smsc_log_info(mainlog,"SUCCESSFULLY REACHED EXIT HANDLER\n"
                      "######################################################################");
    }
    // smsc::util::xml::TerminateXerces();
#ifdef SMPPPDUUSEMEMORYPOOL
    PduBuffer::pool.clean();
    PduBuffer::pool.setLogger(0);
#endif
    smsc::logger::Logger::Shutdown();
}


class SmppThreadTask : public SmppTestTask
{
public:
    SmppThreadTask( ServerIface& server,
                    const std::string sesid,
                    SmppTestCore& core,
                    unsigned speed ) :
    SmppTestTask(core,speed),
    server_(server), sesid_(sesid),
    pass_(0) {}


    Pdu* makePdu()
    {
        std::auto_ptr<Pdu> pdu;
        // switch (++pass_ % 10) {
        // case 0 : {
            DeliverSmPdu* dp = new DeliverSmPdu();
            pdu.reset(dp);
            const char* hello = "hello, world";
            dp->setShortMessage(hello,uint8_t(strlen(hello)));
            dp->setSourceAddr(1,1,"79137654079");
            dp->setDestAddr(0,1,"123");
            dp->optionals.setInt(0x0204,2,1111);
            dp->optionals.setString(0x001e,"MSGRCPT0123456");
        // break;
        /*
        }
        case 1 : break;
        case 2 : break;
        case 3 : break;
        case 4 : break;
        case 5 : break;
        case 6 : break;
        case 7 : break;
        case 8 : break;
        case 9 : break;
        }
         */
        return pdu.release();
    }

    bool getSession( SessionPtr& ptr ) {
        return server_.getSession(sesid_.c_str(),ptr);
    }

private:
    ServerIface& server_;
    std::string  sesid_;
    unsigned     pass_;
};


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
    unsigned nthreads = 1;
    unsigned speed = 0;
    for ( const char** p = argv+1; *p; ++p ) {
        const std::string arg(*p);
        if ( arg == "--threads" ) {
            if (!*++p) return -1;
            nthreads = unsigned(atoi(*p));
            if ( nthreads < 1 ) nthreads = 1;
            if ( nthreads > 1000 ) nthreads = 1000;
        } else if ( arg == "--speed" ) {
            if (!*++p) return -2;
            speed = unsigned(atoi(*p));
        }
    }

    smsc::logger::Logger::Init("server.properties");
    mainlog = smsc::logger::Logger::getInstance("mainlog");
    atexit(atExitHandler);

    smsc_log_info(mainlog,"main started with nthreads=%u speed=%u",
                  nthreads,speed);
#ifdef SMPPPDUUSEMEMORYPOOL
    PduBuffer::pool.setLogger(smsc::logger::Logger::getInstance("pdu.mem"));
#endif

    registerSignalHandler();

    SmppTestCore core(12000);

    SocketMgrConfig cfg;

    SmppServer server( "0.0.0.0", 28830, cfg, &core );
    server.start();
    core.start();

    const char* sessionId = "MSAG1";

    SmeInfo smeInfo;
    smeInfo.systemId = "sme1";
    smeInfo.password = "sme1";
    smeInfo.enabled = true;
    smeInfo.bindMode = BINDMODE_TRANSCEIVER;
    // smeInfo.bindMode = BINDMODE_TRANSRECV;
    smeInfo.nsockets = 1;

    server.addSmeInfo(smeInfo,sessionId);

    smeInfo.systemId = "sme2";
    smeInfo.password = "sme2";
    server.addSmeInfo(smeInfo,sessionId);

    ThreadPool tp;
    if ( speed > 0 && nthreads > 0 ) {
        for ( unsigned i = 0; i < nthreads; ++i ) {
            tp.startTask( new SmppThreadTask(server,
                                             sessionId,
                                             core,
                                             speed) );
        }
    }

    while (!stopping) {
        MutexGuard mg(stopmon);
        stopmon.wait(2000);
        typedef struct stat stat_type;
        stat_type st;
        if ( 0 == stat(stopfile,&st) ) {
            unlink(stopfile);
            stopping = true;
            break;
        }
    }

    server.stop();
    tp.shutdown();
    core.stop();

    smsc_log_info(mainlog,"main finished");
    return 0;
}
