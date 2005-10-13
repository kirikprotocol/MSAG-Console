#ifndef __SCAG_SYSTEM_CORE__
#define __SCAG_SYSTEM_CORE__

#include <sys/types.h>

#include "core/threads/ThreadPool.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "scag/transport/smpp/router/route_manager.h"
#include "util/config/smeman/SmeManConfig.h"
#include "scag/performance.hpp"
#include "sme/SmppBase.hpp"
#include "smeman/smsccmd.h"
#include "scag/stat/StatisticsManager.h"
#include "core/buffers/XHash.hpp"
#include "logger/Logger.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/stat/Statistics.h"
#include "scag/stat/StatisticsManager.h"
#include "sms/sms.h"
#include "scag/transport/smpp/SmppManager.h"
#include "scag/transport/smpp/SmppManagerAdmin.h"
#include "util/crc32.h"

namespace scag
{

using smsc::sms::SMS;
using smsc::core::threads::ThreadedTask;
using smsc::sme::SmeConfig;

using smsc::smeman::SmscCommand;

using namespace smsc::logger;

using scag::config::RouteConfig;
using scag::stat::SmppStatEvent;
using scag::stat::StatisticsManager;
using scag::transport::smpp::router::RouteManager;
using scag::transport::smpp::router::RouteInfo;
using std::string;

namespace StatEvents
{
  const int etSubmitOk     =1;
  const int etSubmitErr    =2;
  const int etDeliveredOk  =3;
  const int etDeliverErr   =4;
  const int etUndeliverable=5;
  const int etRescheduled  =6;
}

class GatewaySme;

class Scag
{
public:
  Scag():stopFlag(false)
  {
    acceptedCounter=0;
    rejectedCounter=0;
    deliveredCounter=0;
    deliverErrCounter=0;
    transOkCounter=0;
    transFailCounter=0;

    startTime=0;
    license.maxsms=0;
    license.expdate=0;
  };
  ~Scag();
  void init();
  void run();
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


  void updatePerformance(int counter)
  {
    MutexGuard g(perfMutex);
    using namespace scag::performance::Counters;
    switch(counter)
    {
      case cntAccepted:    acceptedCounter++;break;
      case cntRejected:    rejectedCounter++;break;
      case cntDelivered:   deliveredCounter++;break;
      case cntDeliverErr:  deliverErrCounter++;break;
      case cntTransOk:     transOkCounter++;break;
      case cntTransFail:   transFailCounter++;break;
    }
  }

  void SaveStats()
  {
    FILE *f=fopen("stats.txt","wt");
    if(f)
    {
      fprintf(f,"%d %lld %lld %lld %lld %lld %lld",time(NULL)-startTime,
        acceptedCounter,
        rejectedCounter,
        deliveredCounter,
        deliverErrCounter,
        transOkCounter,
        transFailCounter
      );
      fclose(f);
    }
  }
  void abortScag()
  {
    SaveStats();
    //MapDialogContainer::getInstance()->abort();
    kill(getpid(),9);
  }

  void dumpScag()
  {
    //MapDialogContainer::getInstance()->abort();
    abort();
  }

  void getPerfData(uint64_t *cnt)
  {
    MutexGuard g(perfMutex);
    cnt[0]=acceptedCounter;
    cnt[1]=rejectedCounter;
    cnt[2]=deliveredCounter;
    cnt[3]=deliverErrCounter;
    cnt[4]=transOkCounter;
    cnt[5]=transFailCounter;
  }

  void InitLicense(const Hash<string>& lic)
  {
    license.maxsms=atoi(lic["MaxSmsThroughput"].c_str());
    int y,m,d;
    sscanf(lic["LicenseExpirationDate"].c_str(),"%d-%d-%d",&y,&m,&d);
    struct tm t={0,};
    t.tm_year=y-1900;
    t.tm_mon=m;
    t.tm_mday=d;
    license.expdate=mktime(&t);
    long hostid;
    std::string ids=lic["Hostids"];
    std::string::size_type pos=0;
    bool ok=false;
    do{
      sscanf(ids.c_str()+pos,"%x",&hostid);
      if(hostid==gethostid())
      {
        ok=true;break;
      }
      pos=ids.find(',',pos);
      if(pos!=std::string::npos)pos++;
    }while(pos!=std::string::npos);
    if(!ok)throw runtime_error("");
    if(smsc::util::crc32(0,lic["Product"].c_str(),lic["Product"].length())!=0x1D5DA434)throw runtime_error("");
    if(license.expdate<time(NULL))
    {
      char x[]=
      {
      'L'^0x4c,'i'^0x4c,'c'^0x4c,'e'^0x4c,'n'^0x4c,'s'^0x4c,'e'^0x4c,' '^0x4c,'E'^0x4c,'x'^0x4c,'p'^0x4c,'i'^0x4c,'r'^0x4c,'e'^0x4c,'d'^0x4c,
      };
      std::string s;
      for(int i=0;i<sizeof(x);i++)
      {
        s+=x[i]^0x4c;
      }
      throw runtime_error(s);
    }
  }

  scag::transport::smpp::SmppManagerAdmin * getSmppManagerAdmin()
  {
      scag::transport::smpp::SmppManagerAdmin * smppManAdmin = &smppMan;

      if(!smppManAdmin)
          throw Exception("Failed casting of SmppManger to SmppManagerAdmin from Scag::getSmppManagerAdmin");

      return smppManAdmin;
  }

protected:

  // AdminCommand
  void processCommand(SmscCommand& cmd);

  void generateAlert(SMS* sms);

  bool stopFlag;
  std::string scagHost;
  int scagPort;
  scag::transport::smpp::SmppManager smppMan;

  scag::performance::PerformanceDataDispatcher perfDataDisp;

  struct LicenseInfo{
    int maxsms;
    time_t expdate;
  }license;

  Mutex perfMutex;

  uint64_t acceptedCounter;
  uint64_t rejectedCounter;
  uint64_t deliveredCounter;
  uint64_t deliverErrCounter;
  uint64_t transOkCounter;
  uint64_t transFailCounter;

  string scAddr;
  string ussdCenterAddr;
  int    ussdSSN;
  time_t startTime;

  int eventQueueLimit;

  smsc::core::threads::ThreadPool tp,tp2;

  friend class StatusSme;

};

}//scag


#endif // __SCAG_SYSTEM_CORE__
