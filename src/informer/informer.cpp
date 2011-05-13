#include <cstdio>
#include <ctime>
#include <memory>
#include <signal.h>
#include <locale.h>

#include "license/check/license.hpp"
#include "logger/Logger.h"
#include "system/smscsignalhandlers.h"
#include "util/config/ConfString.h"
#include "util/crc32.h"
#include "util/findConfigFile.h"
#include "util/regexp/RegExp.hpp"
#include "util/xml/init.h"
#include "version.inc"
#include "informer/main/InfosmeCoreV1.h"
#include "informer/io/InfosmeException.h"

using namespace eyeline::informer;

smsc::logger::Logger* mainlog = 0;

static sigset_t original_signal_mask;
static sigset_t blocked_signals;

#ifdef CHECKCONTENTION
namespace smsc {
namespace core {
namespace synchronization {
unsigned Mutex::contentionLimit = 100000;

void Mutex::reportContention( const char* from,
                              unsigned    howlong,
                              unsigned    oldcount ) const throw()
{
    static smsc::logger::Logger* log_ = 0;
    if ( !log_ ) {
        log_ = smsc::logger::Logger::getInstance("lock");
    }
    smsc_log_debug(log_,"%s(%p) @%s contented by %u@%s for %u usec count=%u/%+d",
                   what, this, from ? from:"unk",
                   unsigned(wasid),
                   wasfrom?wasfrom:"unk",
                   howlong,lockCount,
                   int(lockCount-oldcount));
}

}
}
}
#endif

smsc::core::synchronization::EventMonitor startMon;
bool isStarted = true;

extern "C" void appSignalHandler(int sig)
{
    smsc_log_error(mainlog,"signal handler invoked, sig=%d",sig);
    if ( sig == smsc::system::SHUTDOWN_SIGNAL || sig == SIGINT ) {
        if (isStarted) {
            smsc::core::synchronization::MutexGuard mg(startMon);
            if (isStarted) {
                smsc_log_warn(mainlog," == STOP RECEIVED, set flag to leave main loop");
                isStarted = false;
                startMon.notifyAll();
            }
        }
    }
}


extern "C" void atExitHandler(void)
{
    smsc_log_info(mainlog,"SUCCESSFULLY REACHED EXIT HANDLER, OK");
    smsc::util::xml::TerminateXerces();
    smsc::logger::Logger::Shutdown();
}


/// check license file and return the number of SMS per second limit
unsigned checkLicenseFile()
{
    static time_t licenseFileMTime = 0;
    static unsigned maxsms = 0;
    const std::string licenseFile = smsc::util::findConfigFile("license.ini");
    const std::string licenseSig = smsc::util::findConfigFile("license.sig");
    struct stat fst;
    if (::stat(licenseFile.c_str(),&fst) != 0) {
        throw InfosmeException(EXC_NOTFOUND,"License file is not found: %s",licenseFile.c_str());
    }
    if (fst.st_mtime == licenseFileMTime) {
        return maxsms;
    }

    static const char *lkeys[]=
    {
            "Organization",
            "Hostids",
            "MaxSmsThroughput",
            // "MaxHttpThroughput",
            // "MaxMmsThroughput",
            "LicenseExpirationDate",
            "LicenseType",
            "Product"
    };
    smsc::core::buffers::Hash<std::string> licconfig;
    if (!smsc::license::check::CheckLicense(licenseFile.c_str(),licenseSig.c_str(),
                                            licconfig,lkeys,sizeof(lkeys)/sizeof(lkeys[0])))
    {
        throw InfosmeException(EXC_IOERROR,"Invalid license");
    }
    licenseFileMTime=fst.st_mtime;
    maxsms = unsigned(atoi(licconfig["MaxSmsThroughput"].c_str()));
    if ( maxsms > 10000 ) {
        throw InfosmeException(EXC_CONFIG,"Too big value for MaxSmsThroughput: %u", maxsms);
    }

    time_t expdate;
    {
        int y,m,d;
        sscanf(licconfig["LicenseExpirationDate"].c_str(),"%d-%d-%d",&y,&m,&d);
        struct tm t={0,};
        t.tm_year=y-1900;
        t.tm_mon=m-1;
        t.tm_mday=d;
        expdate = mktime(&t);
    }
    bool ok = false;
    {
        long hostid;
        std::string ids=licconfig["Hostids"];
        std::string::size_type pos=0;
        do {
            sscanf(ids.c_str() + pos,"%lx", &hostid);
            if (hostid == gethostid())
            {
                ok = true;
                break;
            }
            
            pos = ids.find(',', pos);
            if (pos!=std::string::npos) pos++;
        } while(pos!=std::string::npos);
    }
    if (!ok) {
        throw std::runtime_error("code 1");
    }
    if ( smsc::util::crc32(0,licconfig["Product"].c_str(),
                           licconfig["Product"].length())!=0x7fb78bee) {
        // not "informer"
        throw std::runtime_error("code 2");
    }
    if (expdate < time(0)) {
        char x[]=
        {
            'L'^0x4c,'i'^0x4c,'c'^0x4c,'e'^0x4c,'n'^0x4c,'s'^0x4c,'e'^0x4c,' '^0x4c,'E'^0x4c,'x'^0x4c,'p'^0x4c,'i'^0x4c,'r'^0x4c,'e'^0x4c,'d'^0x4c,
        };
        std::string s;
        for (unsigned i=0;i<sizeof(x);i++)
        {
            s+=x[i]^0x4c;
        }
        throw std::runtime_error(s);
    }
    return maxsms;
}


int main( int argc, const char** argv )
{
    bool selftest = false;
    bool archive = false;
    try {

        for ( const char** arg = argv+1; *arg != 0; ++arg ) {

            if (!strcmp(*arg,"-h") || !strcmp(*arg,"--help")) {
                printf("%s [options]\n",argv[0]);
                printf(" Options:\n");
                printf(" -h --help   \tThis help\n");
                printf(" -v --version\tShow program version\n");
                printf("    --selftest\tExecute self-test upon program startup\n");
                printf("    --archive\tRun in archive regime\n");
                return 0;
            }

            if (!strcmp(*arg, "-v") || !strcmp(*arg,"--version")) {
                printf("%s\n", getStrVersion());
                return 0;
            }

            if (!strcmp(*arg,"--selftest")) {
                selftest = true;
                continue;
            }

            if (!strcmp(*arg,"--archive")) {
                archive = true;
                continue;
            }

        }

        // infrastructure
        smsc::util::regexp::RegExp::InitLocale();
        setlocale(LC_ALL,"");

        do {
            char* loc = setlocale(LC_CTYPE,NULL);
            loc = strchr(loc,'.');
            if (loc) {
                ++loc;
                if ( strncmp(loc,"UTF-8",5) == 0 ) {
                    // ok
                    break;
                }
            }
            throw InfosmeException(EXC_CONFIG,"LC_ALL/LC_CTYPE are not set correctly (UTF-8 needed)");
        } while (false);

        smsc::logger::Logger::Init();
        smsc::util::xml::initXerces();

        mainlog = smsc::logger::Logger::getInstance("main");
        smsc_log_info(mainlog,"\n"
                      "------------------------------------------------------------------------");
        smsc_log_info(mainlog,"Starting up %s",getStrVersion());

        try {

            std::auto_ptr< InfosmeCoreV1 > core;

            atexit( atExitHandler );

            // block all signals
            sigfillset(&blocked_signals);
            sigdelset(&blocked_signals, SIGKILL);
            sigdelset(&blocked_signals, SIGALRM);
            sigdelset(&blocked_signals, SIGSEGV);
            sigdelset(&blocked_signals, SIGBUS);
            sigdelset(&blocked_signals, SIGFPE);
            sigdelset(&blocked_signals, SIGILL);
            pthread_sigmask(SIG_SETMASK, &blocked_signals, &original_signal_mask);

            // setting signal handlers
            sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
            sigset(SIGINT, appSignalHandler);
            sigset(SIGPIPE, SIG_IGN);

            // license
            const unsigned maxsms = checkLicenseFile();

            core.reset( new InfosmeCoreV1(maxsms) );
            core->init( archive );

            // enter main loop
            core->start();
            pthread_sigmask(SIG_SETMASK, &original_signal_mask, 0);

            if ( selftest ) {
                try {
                    core->selfTest();
                } catch (std::exception& e ) {
                    smsc_log_error(mainlog,"self test failed, exc: %s",e.what());
                    // isStarted = false;
                }
            }

            while ( isStarted && !getCS()->isStopping() ) {
                smsc::core::synchronization::MutexGuard mg(startMon);
                startMon.wait(1000);
            }
            smsc_log_error(mainlog,"leaving main loop, stopping core");
            core->stop();
            
        } catch ( std::exception& e ) {
            smsc_log_error(mainlog,"EXCEPTION, exc: %s",e.what());
            smsc::util::xml::TerminateXerces();
            throw;
        }
        smsc::util::xml::TerminateXerces();

    } catch ( std::exception& e ) {
        fprintf(stderr,"EXCEPTION: %s\n",e.what());
        return -1;
    } catch ( ... ) {
        fprintf(stderr,"UNKNOWN EXCEPTION\n");
        return -1;
    }
    return 0;
}
