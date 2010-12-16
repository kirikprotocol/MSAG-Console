#ifndef __SCAG_SYSTEM_CORE2__
#define __SCAG_SYSTEM_CORE2__

#include <sys/types.h>

#include "core/buffers/XHash.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "logger/Logger.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/stat/base/Statistics2.h"
// #include "transport/http/Managers.h"
// #include "transport/http/HttpProcessor.h"
#include "scag/transport/smpp/router/route_manager.h"
#include "scag/transport/smpp/base/SmppManager2.h"
// #include "scag/transport/smpp/SmppManagerAdmin2.h"
#include "scag/util/Reffer.h"
#include "smeman/smsccmd.h"
#include "sms/sms.h"
// #include "scag/sessions/base/SessionManager2.h"
//#include "sme/SmppBase.hpp"
//#include "util/config/smeman/SmeManConfig.h"

#ifdef SNMP
#include "scag/snmp/SnmpTrapThread.h"
#endif

namespace scag2
{

using smsc::sms::SMS;
using smsc::core::threads::ThreadedTask;
//using smsc::sme::SmeConfig;

//using smsc::smeman::SmscCommand;

using namespace smsc::logger;

using scag::config::RouteConfig;
using stat::SmppStatEvent;
using scag2::transport::smpp::router::RouteManager;
using scag2::transport::smpp::router::RouteInfo;
using std::string;
using scag::util::Reffer;
using scag::util::RefferGuard;

/*namespace StatEvents
{
  const int etSubmitOk     =1;
  const int etSubmitErr    =2;
  const int etDeliveredOk  =3;
  const int etDeliverErr   =4;
  const int etUndeliverable=5;
  const int etRescheduled  =6;
}

class GatewaySme;
*/
class Scag
{
public:

    Scag();
    ~Scag();

    void init( unsigned mynode );
//  void run();
  void stop(){stopFlag=true;}
  //void mainLoop();
  void shutdown();


  void unregisterSmeProxy(const string& sysid)
  {
    // Wait for SmppManager will be ready
    /*MutexGuard mg(gatewaySwitchMutex);

    uint8_t uid = 0;

    SmeRecord* p = (SmeRecord*)getSmeProxy(sysid);
    GatewaySme* gwsme = dynamic_cast<GatewaySme*>(p->proxy);

    if(gwsme){
        smeman.unregSmsc(sysid);
        uid = gwsme->getPrefix();

        gwsme->Release();

        if(uid){
            gwSmeMap[uid]->Release();
            gwSmeMap[uid] = 0;
        }
    }*/
  }


  void abortScag()
  {
    //MapDialogContainer::getInstance()->abort();
    kill(getpid(),9);
  }

  void dumpScag()
  {
    //MapDialogContainer::getInstance()->abort();
    abort();
  }

    /*
  RefferGuard<RouteManager> getTestRouterInstance()
  {
    MutexGuard g(routerSwitchMutex);
    return RefferGuard<RouteManager>(testRouter_);
  }

  void ResetTestRouteManager(RouteManager* manager)
  {
    MutexGuard g(routerSwitchMutex);
    if ( testRouter_ ) testRouter_->Release();
    testRouter_ = new Reffer<RouteManager>(manager);
  }

  void reloadTestRoutes(const RouteConfig& rcfg);
     */

    transport::smpp::SmppManagerAdmin* getSmppManagerAdmin()
    {
        return & transport::smpp::SmppManager::Instance();
    }

protected:

  // AdminCommand
  //void processCommand(SmscCommand& cmd);

  void generateAlert(SMS* sms);

  bool stopFlag;
  std::string scagHost;
  int scagPort;
//  scag::transport::smpp::SmppManager smppMan;
//  scag::transport::http::HttpManager httpMan;

/*  Mutex perfMutex;

  uint64_t acceptedCounter;
  uint64_t rejectedCounter;
  uint64_t deliveredCounter;
  uint64_t deliverErrCounter;
  uint64_t transOkCounter;
  uint64_t transFailCounter;
*/
  string scAddr;
  string ussdCenterAddr;
  int    ussdSSN;
  time_t startTime;

  //smsc::core::threads::ThreadPool tp,tp2;

  // Mutex routerSwitchMutex;
  // Reffer<RouteManager>* testRouter_;

  friend class StatusSme;

#ifdef SNMP
    // std::auto_ptr< sessions::SessionManager >  sessman_;
    std::auto_ptr< snmp::SnmpWrapper > snmp_;
    std::auto_ptr< snmp::SnmpTrapThread > snmpthread_;
#endif

private:
    bool pvssInited_;
    bool lcmInited_;
    bool billInited_;
    bool httpInited_;
    bool smppInited_;
    bool sessInited_;
    bool statInited_;
};

} // scag2

#endif // __SCAG_SYSTEM_CORE__
