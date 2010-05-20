#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

#ident "@(#)$Id$"

#include <core/threads/ThreadPool.hpp>
#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/recoder/recode_dll.h>
#include <util/smstext.h>

#ifndef INFOSME_NO_DATAPROVIDER
#include <db/DataSourceLoader.h>
#endif
#include <core/synchronization/EventMonitor.hpp>

#include <system/smscsignalhandlers.h>
#include <sme/SmppBase.hpp>
#include <sms/sms.h>
#include <util/xml/init.h>

#include <admin/service/Component.h>
#include <admin/service/ComponentManager.h>
#include <admin/service/ServiceSocketListener.h>
#include <system/status.h>

#include <util/timeslotcounter.hpp>

#include "TaskProcessor.h"
#include "InfoSmeComponent.h"
#include "InfoSmeMessageSender.h"
#include "util/mirrorfile/mirrorfile.h"

#include <sms/sms.h>
#include <util/config/route/RouteStructures.h>
#include <util/config/route/RouteConfig.h>

// #include <util/config/region/RegionsConfig.hpp>
#include <util/config/region/Region.hpp>
#include <util/config/region/RegionFinder.hpp>
#include "infosme/TaskLock.hpp"
#include "SmscConnector.h"

#include "version.inc"
#include "util/regexp/RegExp.hpp"
#include "util/config/ConfString.h"
#include "util/findConfigFile.h"
#include "util/crc32.h"
#include "license/check/license.hpp"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;
using namespace smsc::system;

using namespace smsc::admin;
using namespace smsc::admin::service;

using namespace smsc::infosme;
using smsc::util::TimeSlotCounter;
using smsc::core::synchronization::EventMonitor;

//const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;
//const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

static smsc::logger::Logger *logger = 0;

static ServiceSocketListener* adminListener = 0;
static bool bAdminListenerInited = false;

static Event infoSmeWaitEvent;
static Event infoSmeReady;
static int   infoSmeReadyTimeout = 60000;

static Mutex needStopLock;
static Mutex needReconnectLock;

static sig_atomic_t  infoSmeIsStopped    = 0;
static bool  bInfoSmeIsConnected  = false;
static bool  bInfoSmeIsConnecting = false;

static int signaling_ReadFd = -1, signaling_WriteFd = -1;
static int reconnect_ReadFd = -1, reconnect_WriteFd = -1;

static sigset_t blocked_signals, original_signal_mask;

static void setNeedStop()
{
  infoSmeIsStopped = 1;
}

static bool isNeedStop() {
  sigprocmask(SIG_SETMASK, &original_signal_mask, NULL); // unlock all signals and deliver any pending signals
  sigprocmask(SIG_SETMASK, &blocked_signals, NULL); // lock all signals.
  return infoSmeIsStopped == 1;
}

static void setNeedReconnect() {
    MutexGuard gauard(needReconnectLock);
    smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.infosme.InfoSme");
    smsc_log_info(logger, "setNeedReconnect:: Enter it");
    if ( bInfoSmeIsConnected ) {
      bInfoSmeIsConnected = false;
      unsigned char oneByte=0;
      smsc_log_info(log, "setNeedReconnect:: write to reconnect_WriteFd");
      write(reconnect_WriteFd, &oneByte, sizeof(oneByte));
    }
}

static void setConnected() {
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.infosme.InfoSme");
  MutexGuard gauard(needReconnectLock);
  bInfoSmeIsConnected=true;
  smsc_log_info(log, "setConnected:: bInfoSmeIsConnected=true");
}

extern bool isMSISDNAddress(const char* string)
{
    try { Address converted(string); } catch (...) { return false;}
    return true;
}
static int  maxMessagesPerSecond     = 50;

#include "TrafficControl.hpp"

void TrafficControl::incOutgoing( unsigned nchunks )
{
  MutexGuard guard(trafficMonitor);
  outgoing.Inc( nchunks );
  if (TrafficControl::stopped) return;

  int out = outgoing.Get();

  bool firstTime = true;
  while (out >= maxMessagesPerSecond) {
    // traffic limit reached
    if (firstTime) {
        smsc_log_info(logger, "wait limit (out=%d, max=%d)",
                      out, maxMessagesPerSecond);
        firstTime = false;
    }
    trafficMonitor.wait(std::max(1000/maxMessagesPerSecond,1));
    out = outgoing.Get();
  }
}

void TrafficControl::incIncoming()
{
  MutexGuard guard(trafficMonitor);
  incoming.Inc();
  if (TrafficControl::stopped) return;
  int out = outgoing.Get();// int inc = incoming.Get();
  //int difference = out-inc;
  bool trafficLimitOk = (out < maxMessagesPerSecond);

  if (trafficLimitOk) {
    trafficMonitor.notifyAll();
  }
}

void TrafficControl::stopControl()
{
  MutexGuard guard(trafficMonitor);
  TrafficControl::stopped = true;
  trafficMonitor.notifyAll();
}

void TrafficControl::startControl()
{
  MutexGuard guard(trafficMonitor);
  TrafficControl::stopped = false;
}

EventMonitor         TrafficControl::trafficMonitor;
TimeSlotCounter<int> TrafficControl::incoming(1, 1);
TimeSlotCounter<int> TrafficControl::outgoing(1, 1);
bool                 TrafficControl::stopped = false;

extern "C" void appSignalHandler(int sig)
{
  if (sig==smsc::system::SHUTDOWN_SIGNAL || sig==SIGINT) {
    if ( signaling_WriteFd > -1 ) close (signaling_WriteFd);
    setNeedStop();
  }
}

extern "C" void atExitHandler(void)
{
    // delete regionsConfig;
    smsc::util::xml::TerminateXerces();
    smsc::logger::Logger::Shutdown();
}

unsigned checkLicenseFile()
{
    static time_t licenseFileMTime = 0;
    static unsigned maxsms = 0;
    const std::string licenseFile = smsc::util::findConfigFile("license.ini");
    const std::string licenseSig = smsc::util::findConfigFile("license.sig");
    struct stat fst;
    if (::stat(licenseFile.c_str(),&fst) != 0) {
        throw smsc::util::Exception("License file is not found: %s", licenseFile.c_str());
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
        throw smsc::util::Exception("Invalid license");
    }
    licenseFileMTime=fst.st_mtime;
    maxsms = unsigned(atoi(licconfig["MaxSmsThroughput"].c_str()));
    if ( maxsms > 10000 ) {
        throw smsc::util::Exception("Too big value for MaxSmsThroughput: %u", maxsms);
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
        // not "infosme"
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


int main(int argc, char** argv)
{
  try {
    if ( argc > 1 && !strcmp(argv[1], "-v") ) {
      printf("%s\n", getStrVersion());
      return 0;
    }

    int fds[2];
    pipe(fds);
    signaling_ReadFd = fds[0]; signaling_WriteFd = fds[1]; 
    pipe(fds);
    reconnect_ReadFd = fds[0]; reconnect_WriteFd = fds[1];

    int resultCode = 0;

      smsc::util::regexp::RegExp::InitLocale();

    Logger::Init();
    logger = Logger::getInstance("smsc.infosme.InfoSme");
    smsc_log_info(logger, "Starting up %s", getStrVersion());

    std::auto_ptr<ServiceSocketListener> adml(new ServiceSocketListener());
    adminListener = adml.get();

    atexit(atExitHandler);

    sigset_t set;
    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, &original_signal_mask);
    sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
    sigset(SIGINT, appSignalHandler);
    sigset(SIGPIPE, SIG_IGN);

    try
    {
        // checking license file
        maxMessagesPerSecond = checkLicenseFile();

        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

#ifndef INFOSME_NO_DATAPROVIDER
        ConfigView dsConfig(manager, "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);
#endif

        ConfigView tpConfig(manager, "InfoSme");

        // maxMessagesPerSecond++;

        /*
        try { maxMessagesPerSecond = tpConfig.getInt("maxMessagesPerSecond"); } catch (...) {};
        if (maxMessagesPerSecond <= 0) {
            maxMessagesPerSecond = 50;
            smsc_log_warn(logger, "Parameter 'maxMessagesPerSecond' value is invalid. Using default %d",
                          maxMessagesPerSecond);
        }
        if (maxMessagesPerSecond > 100) {
            smsc_log_warn(logger, "Parameter 'maxMessagesPerSecond' value '%d' is too big. "
                          "The preffered max value is 100", maxMessagesPerSecond);
        }
         */

        {
          smsc::util::config::ConfString fnStr(tpConfig.getString("storeLocation"));
          std::string fn = fnStr.str();
          if(fn.length() && *fn.rbegin()!='/')
          {
            fn+='/';
          }
          fn+="taskslock.bin";
          TaskLock::Init(fn.c_str());
        }

        TaskProcessor processor(&tpConfig);

        InfoSmeMessageSender messageSender( processor );
        messageSender.reloadSmscAndRegions( manager );

        sigfillset(&blocked_signals);
        sigdelset(&blocked_signals, SIGKILL);
        sigdelset(&blocked_signals, SIGALRM);
        sigdelset(&blocked_signals, SIGSEGV); sigdelset(&blocked_signals, SIGBUS);
        sigdelset(&blocked_signals, SIGFPE); sigdelset(&blocked_signals, SIGILL);

        sigprocmask(SIG_SETMASK, &blocked_signals, &original_signal_mask);

        ConfigView adminConfig(manager, "InfoSme.Admin");
        adminListener->init(smsc::util::config::ConfString(adminConfig.getString("host")).c_str(),
                            adminConfig.getInt("port"));
        bAdminListenerInited = true;


        InfoSmeComponent admin(processor);
        ComponentManager::registerComponent(&admin);
        adminListener->Start();

        bool haveSysError=false;

        messageSender.start();

        while (!isNeedStop() && !haveSysError)
        {
          // after call to isNeedStop() was completed all signals is locked.
          // any thread being started from this point has signal mask with all signals locked 


            smsc_log_info(logger, "Starting InfoSME ... ");

            //try
            //{
                //listener.setSyncTransmitter(session.getSyncTransmitter());
                //listener.setAsyncTransmitter(session.getAsyncTransmitter());

                bInfoSmeIsConnecting = true;
                infoSmeReady.Wait(0);
                TrafficControl::startControl();
                //session.connect();
                processor.assignMessageSender(&messageSender);
                processor.Start();
                bInfoSmeIsConnecting = false;
                infoSmeReady.Signal();
                setConnected();
            //} 
            /*catch (SmppConnectException& exc)
            {
                const char* msg = exc.what();
                smsc_log_error(logger, "Connect to SMSC failed. Cause: %s", (msg) ? msg:"unknown");
                bInfoSmeIsConnecting = false;
                if (exc.getReason() == SmppConnectException::Reason::bindFailed) throw;
                sleep(cfg.timeOut);
                session.close();
                continue;
            }*/
            smsc_log_info(logger, "Connected.");
            sigprocmask(SIG_SETMASK, &original_signal_mask, NULL); // unlock all signals and deliver any pending signals
            int st;
            fd_set rFdSet;
            FD_ZERO(&rFdSet);
            FD_SET(signaling_ReadFd, &rFdSet); FD_SET(reconnect_ReadFd, &rFdSet); 
            if ( (st=::select(std::max(signaling_ReadFd, reconnect_ReadFd)+1, &rFdSet, NULL, NULL, NULL)) > 0 ) {
              smsc_log_info(logger, "select return %d", st);
              if ( FD_ISSET(reconnect_ReadFd, &rFdSet) ) {
                unsigned char oneByte;
                if ( read(reconnect_ReadFd, &oneByte, sizeof(oneByte)) != sizeof(oneByte) )
                  haveSysError = true;
              }
            } else if ( st == -1 ) {
              smsc_log_info(logger, "select return -1: errno=%d", errno);
              if ( errno != EINTR )
                haveSysError = true;
            }

            smsc_log_info(logger, "Disconnecting from SMSC ...");
            TrafficControl::stopControl();
            processor.Stop();
            processor.assignMessageSender(0);

            messageSender.stop();
            // sleep for 2 secs.
            struct timeval sleepTimeout;
            sleepTimeout.tv_sec = 2;
            sleepTimeout.tv_usec = 0;
            ::select(0, NULL, NULL, NULL, &sleepTimeout);
        }
    }
    catch (SmppConnectException& exc)
    {
        if (exc.getReason() == SmppConnectException::Reason::bindFailed)
            smsc_log_error(logger, "Failed to bind InfoSme. Exiting.");
        resultCode = -1;
    }
    catch (ConfigException& exc)
    {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting.", exc.what());
        resultCode = -2;
    }
    catch (Exception& exc)
    {
        smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    }
    catch (exception& exc)
    {
        smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
        resultCode = -4;
    }
    catch (...)
    {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
        resultCode = -5;
    }

    if (bAdminListenerInited)
    {
        adminListener->shutdown();
        adminListener->WaitFor();
    }

#ifndef INFOSME_NO_DATAPROVIDER
    DataSourceLoader::unload();
#endif
    return resultCode;
  } catch (std::exception& ex) {
    fprintf(stderr, "catch unexpected exception[%s]\n",ex.what());
    return -1;
  }
}
