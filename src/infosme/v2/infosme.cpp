#include <cstdio>
#include <cstdlib>
#include <signal.h>

#ident "@(#)$Id$"

#include "logger/Logger.h"
#include "system/smscsignalhandlers.h"
#include "util/xml/init.h"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "db/DataSourceLoader.h"
#include "admin/service/ComponentManager.h"

#include "TaskProcessor.h"
#include "InfoSmeComponent.h"

// #include "core/threads/ThreadPool.hpp"
// #include "core/buffers/Array.hpp"
// #include "core/buffers/Hash.hpp"
// #include "util/recoder/recode_dll.h"
// #include "util/smstext.h"
// #include "core/synchronization/EventMonitor.hpp"

/*
#include <sme/SmppBase.hpp>
#include <sms/sms.h>
#include <util/xml/init.h>

#include <admin/service/Component.h>
#include <admin/service/ServiceSocketListener.h>
#include <system/status.h>

#include <util/timeslotcounter.hpp>

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
*/

#include "version.inc"

/*
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
*/

//const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;
//const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

namespace {

smsc::logger::Logger *logger = 0;
smsc::core::synchronization::EventMonitor stopMon;
bool needStop = false;
bool needReload = false;

}

/*
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
*/

/*
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
    MutexGuard guard(needReconnectLock);
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
  MutexGuard guard(needReconnectLock);
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

void TrafficControl::incOutgoing()
{
  MutexGuard guard(trafficMonitor);
  outgoing.Inc();
  if (TrafficControl::stopped) return;

  int out = outgoing.Get();

  while (out >= maxMessagesPerSecond) {
    // traffic limit reached
    smsc_log_info(logger, "wait limit (out=%d, max=%d)",
                  out, maxMessagesPerSecond);
    trafficMonitor.wait(1000/maxMessagesPerSecond);
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

*/

extern "C" static void appSignalHandler(int sig)
{
    smsc_log_warn(logger,"signal %d handled", sig );
    if (sig==smsc::system::SHUTDOWN_SIGNAL || sig==SIGINT) {
        // if ( signaling_WriteFd > -1 ) close(signaling_WriteFd);
        // setNeedStop();
        MutexGuard mg(stopMon);
        needStop = true;
        stopMon.notify();
    } else if ( sig == SIGHUP ) {
        smsc_log_info(logger,"Reloading config");
        MutexGuard mg(stopMon);
        needReload = true;
        stopMon.notify();
    }
}

extern "C" static void atExitHandler(void)
{
    // delete regionsConfig;
    smsc::util::xml::TerminateXerces();
    smsc::logger::Logger::Shutdown();
}

extern "C" static void registerSignalHandlers( sigset_t* blocked_signals )
{
    if ( ! blocked_signals ) return;

    sigfillset(blocked_signals);
    sigdelset(blocked_signals, SIGBUS);
    sigdelset(blocked_signals, SIGFPE);
    sigdelset(blocked_signals, SIGILL);
    sigdelset(blocked_signals, SIGSEGV);
    sigdelset(blocked_signals, SIGINT);
    sigprocmask(SIG_SETMASK, blocked_signals, NULL );

    // sigdelset(blocked_signals, SIGUSR2);
    sigdelset(blocked_signals, SIGALRM);
    sigdelset(blocked_signals, SIGABRT);
    sigdelset(blocked_signals, smsc::system::SHUTDOWN_SIGNAL);
    sigdelset(blocked_signals, SIGHUP);

    // set handlers
    // sigset(SIGPIPE, SIG_IGN);
    // sigset(SIGUSR2, appSignalHandler );
    sigset(SIGINT, appSignalHandler);
    sigset(SIGFPE, appSignalHandler);
    sigset(SIGILL, appSignalHandler);

    signal(SIGABRT, appSignalHandler);
    sigset(SIGALRM, appSignalHandler);
    sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
    sigset(SIGHUP, appSignalHandler);
}


int main(int argc, char** argv)
{
    sigset_t signalset;
    int resultCode = 0;
    try {

        if ( argc > 1 && !strcmp(argv[1], "-v") ) {
            printf("%s\n", getStrVersion());
            return 0;
        }

        /*
        int fds[2];
        pipe(fds);
        signaling_ReadFd = fds[0]; signaling_WriteFd = fds[1]; 
        pipe(fds);
        reconnect_ReadFd = fds[0]; reconnect_WriteFd = fds[1];
         */

        bool bAdminListenerInited = false;

        Logger::Init();
        logger = Logger::getInstance("is2.main");
        smsc_log_info(logger, "Starting up %s", getStrVersion());

        std::auto_ptr<smsc::admin::service::ServiceSocketListener> adminListener
            (new smsc::admin::service::ServiceSocketListener("is2.adsock"));

        atexit(atExitHandler);
        registerSignalHandlers(&signalset);

        try {

            smsc::util::config::Manager::init("config.xml");
            smsc::util::config::Manager& manager = 
                smsc::util::config::Manager::getInstance();

            smsc::util::config::ConfigView dsConfig(manager, "StartupLoader");
            smsc::db::DataSourceLoader::loadup(&dsConfig);

            smsc::util::config::ConfigView tpConfig(manager, "InfoSme");

            /*
            maxMessagesPerSecond++;

            try {
                maxMessagesPerSecond = tpConfig.getInt("maxMessagesPerSecond"); 
            } catch (...) {}
            if (maxMessagesPerSecond <= 0) {
                smsc_log_warn(logger, "Parameter 'maxMessagesPerSecond' value is invalid. Using default %d",
                              maxMessagesPerSecond);
                maxMessagesPerSecond = 50;
            }
            if (maxMessagesPerSecond > 100) {
                smsc_log_warn(logger, "Parameter 'maxMessagesPerSecond' value '%d' is too big. "
                              "The preffered max value is 100", maxMessagesPerSecond);
                maxMessagesPerSecond = 100;
            }
             */

            /*
            {
                std::string fn = tpConfig.getString("storeLocation");
                if(fn.length() && *fn.rbegin()!='/')
                {
                    fn+='/';
                }
                fn += "taskslock.bin";
                // smsc::infosme2::TaskLock::Init(fn.c_str());
            }
             */

            smsc::infosme2::TaskProcessor processor;
            processor.init(&tpConfig);

            /*
            InfoSmeMessageSender messageSender( processor );
            messageSender.reloadSmscAndRegions( manager );
             */

            smsc::util::config::ConfigView adminConfig(manager, "InfoSme.Admin");
            adminListener->init(adminConfig.getString("host"), adminConfig.getInt("port"));
            bAdminListenerInited = true;

            smsc::infosme2::InfoSmeComponent admin(processor);
            smsc::admin::service::ComponentManager::registerComponent(&admin);
            processor.startTaskProcessor();
            adminListener->Start();

            {
                MutexGuard mg(stopMon);
                sigset_t oldmask;
                sigprocmask(SIG_SETMASK,&signalset,&oldmask);

                // awaiting stop
                while ( !needStop ) {
                    if ( needReload ) {
                        smsc_log_warn(logger,"config reload is not impl yet" );
                        needReload = false;
                    }
                    stopMon.wait(1000);
                }
                sigprocmask(SIG_SETMASK,&oldmask,NULL);
            }

            /*
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
             */
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
            /*
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
             */
            // processor.Stop();
            // processor.assignMessageSender(0);

            // messageSender.stop();
            // sleep for 2 secs.
            // struct timeval sleepTimeout;
            // sleepTimeout.tv_sec = 2;
            // sleepTimeout.tv_usec = 0;
            // ::select(0, NULL, NULL, NULL, &sleepTimeout);

        } catch (smsc::util::config::ConfigException& exc ) {
            smsc_log_error(logger, "ConfigException: %s", exc.what());
            resultCode = -2;
        } catch (std::exception& exc ) {
            smsc_log_error(logger, "Top level Exception: %s", exc.what());
            resultCode = -3;
        } catch (...) {
            smsc_log_error(logger, "Top level unknown exception");
            resultCode = -5;
        }

        if (bAdminListenerInited)
        {
            adminListener->shutdown();
            adminListener->WaitFor();
        }

        smsc::db::DataSourceLoader::unload();
        return resultCode;

    } catch (std::exception& ex) {
        fprintf(stderr, "catch unexpected exception[%s]\n",ex.what());
        return -1;
    }
}
