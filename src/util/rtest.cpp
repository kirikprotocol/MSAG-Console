#include <stdio.h>
#include "util/config/route/RouteConfig.h"
#include "util/config/smeman/SmeManConfig.h"
#include "router/route_manager.h"
#include "util/Timer.hpp"
#include "smeman/smeman.h"
#include "util/regexp/RegExp.hpp"
#include "logger/Logger.h"

using namespace smsc::router;
using namespace smsc::smeman;

namespace smsc{
namespace system{
extern void loadRoutes(RouteManager* rm,const smsc::util::config::route::RouteConfig& rc,bool traceit=false);
}
}

int main(int argc,char* argv[])
{
  if(argc!=3)return 0;

  smsc::logger::Logger::Init("logger.properties");

  RouteManager rm;
  smsc::smeman::SmeManager smeman;

  smsc::util::config::smeman::SmeManConfig smemanconfig;
  smemanconfig.load("sme.xml");


  {
    smsc::util::config::smeman::SmeManConfig::RecordIterator i=smemanconfig.getRecordIterator();
    using namespace smsc::util::regexp;
    RegExp re;
    while(i.hasRecord())
    {
      smsc::util::config::smeman::SmeRecord *rec;
      i.fetchNext(rec);
      SmeInfo si;
      if(rec->rectype==smsc::util::config::smeman::SMPP_SME)
      {
        si.typeOfNumber=rec->recdata.smppSme.typeOfNumber;
        si.numberingPlan=rec->recdata.smppSme.numberingPlan;
        si.interfaceVersion=rec->recdata.smppSme.interfaceVersion;
        si.rangeOfAddress=rec->recdata.smppSme.addrRange;
        si.systemType=rec->recdata.smppSme.systemType;
        si.password=rec->recdata.smppSme.password;
        si.systemId=rec->smeUid;
        si.timeout = rec->recdata.smppSme.timeout;
        si.wantAlias = rec->recdata.smppSme.wantAlias;
        si.forceDC = rec->recdata.smppSme.forceDC;
        si.proclimit=rec->recdata.smppSme.proclimit;
        si.schedlimit=rec->recdata.smppSme.schedlimit;
        si.receiptSchemeName= rec->recdata.smppSme.receiptSchemeName;
        if(si.rangeOfAddress.length() && !re.Compile(si.rangeOfAddress.c_str(),OP_OPTIMIZE|OP_STRICT))
        {
          //smsc_log_error(log, "Failed to compile rangeOfAddress for sme %s",si.systemId.c_str());
        }
        //__trace2__("INIT: addSme %s(to=%d,wa=%s)",si.systemId.c_str(),si.timeout,si.wantAlias?"true":"false");
        //si.hostname=rec->recdata->smppSme.
        si.disabled=rec->recdata.smppSme.disabled;
        using namespace smsc::util::config::smeman;
        switch(rec->recdata.smppSme.mode)
        {
          case MODE_TX:si.bindMode=smeTX;break;
          case MODE_RX:si.bindMode=smeRX;break;
          case MODE_TRX:si.bindMode=smeTRX;break;
        };

        try{
          smeman.addSme(si);
        }catch(...)
        {
          //smsc_log_warn(log, "UNABLE TO REGISTER SME:%s",si.systemId.c_str());
        }
      }
    }
  }

  smsc::util::config::route::RouteConfig rc;

  rm.assign(&smeman);


  rc.load("routes.xml");
  smsc::system::loadRoutes(&rm,rc);



  Address src(argv[1]);
  Address dst(argv[2]);
  smsc::smeman::SmeProxy* prx;
  int idx;
  int cnt=0;
  TIMETHIS("router->lookup",1000000)
  {
    RouteInfo ri;
    if(rm.lookup(src,dst,prx,&idx,&ri))
    {
      cnt++;
    }
  }
  printf("cnt=%d\n",cnt);

  return 0;
}
