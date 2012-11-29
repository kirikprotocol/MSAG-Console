#include <time.h>
#include <stdlib.h>
#include "logger/Logger.h"
#include "util/signal.hpp"
// #include "util/xml/init.h"
#include "SmppClient.h"
#include "core/threads/ThreadPool.hpp"
#include "eyeline/smpp/pdu/Pdu.h"
#include "eyeline/smpp/pdu/DeliverSmPdu.h"
#include "eyeline/smpp/pdu/SubmitSmPdu.h"
#include "eyeline/smpp/transport/SmppTestCore.h"
#include "eyeline/smpp/transport/SmppTestTask.h"

using namespace eyeline::smpp;
using namespace smsc::core::synchronization;
using namespace smsc::core::threads;

EventMonitor stopmon;
bool stopping = false;
const char* stopfile = "client.stop";
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
    SmppThreadTask( SessionPtr session,
                    SmppTestCore& core,
                    unsigned speed ) :
    SmppTestTask(core,speed),
    session_(session),
    pass_(0) {}


    Pdu* makePdu()
    {
        static char payload[512];
        static bool payloadfilled = false;
        if (!payloadfilled) {
            memset(payload,'0',sizeof(payload));
            const char* pfx = "hello world how are you doing, it is a test payload which may be filled into smpp pdu.";
            memcpy(payload,pfx,strlen(pfx));
            payloadfilled = true;
        }
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
            // dp->optionals.setBinary(0x424,uint16_t(sizeof(payload)),payload);
        /*
            break;
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
        ptr = session_; 
        return ptr.get();
    }

private:
    SessionPtr session_;
    unsigned   pass_;
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
    unsigned speed = 1;
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

    smsc::logger::Logger::Init("client.properties");
    mainlog = smsc::logger::Logger::getInstance("mainlog");
    atexit(atExitHandler);

    smsc_log_info(mainlog,"main started with nthreads=%u speed=%u",
                  nthreads,speed);
#ifdef SMPPPDUUSEMEMORYPOOL
    PduBuffer::pool.setLogger(smsc::logger::Logger::getInstance("pdu.mem"));
#endif

    registerSignalHandler();

    SmppTestCore core( 12000 );

    SocketMgrConfig cfg;
    SmppClient client(cfg,&core);
    client.start();
    core.start();

    const char* sessionId = "SMSC1";

    ClientSmeInfo info;
    info.enabled = true;
    // info.host = "0.0.0.0";
    info.host = "localhost";
    info.port = 28830;
    info.systemId = "sme1";
    info.password = "sme1";
    info.bindMode = BINDMODE_TRANSCEIVER;
    // info.bindMode = BINDMODE_TRANSRECV;
    info.nsockets = 1;

    client.addSmeInfo( info, sessionId );

    info.systemId = "sme2";
    info.password = "sme2";
    info.nsockets = 2;
    client.addSmeInfo( info, sessionId );

    ThreadPool tp;
    {
        SessionPtr session;
        if ( speed > 0 && 
             nthreads > 0 &&
             client.getSession(sessionId,session) ) {
            for ( unsigned i = 0; i < nthreads; ++i ) {
                tp.startTask( new SmppThreadTask(session,
                                                 core,
                                                 speed) );
            }
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

    client.stop();
    tp.shutdown();
    core.stop();

    smsc_log_info(mainlog,"main finished");
    // PduBuffer::pool.setLogger(0);
    return 0;
}
