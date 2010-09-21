
#include "util/config/route/RouteConfig.h"
#include "router/route_manager.h"
#include "smeman/smeman.h"
#include "sms/sms.h"
#include <iostream>
#include <string>
#include <stdexcept>

//#define LOAD_ROUTES_TEST

using namespace smsc::util::config::route;
using namespace smsc::sms;
using namespace std;
using namespace smsc::router;
using namespace smsc::smeman;

namespace smsc {
namespace system {

static inline void makeAddress(Address& addr,const string& mask)
{
  addr=Address(mask.c_str());
}

void loadRoutes(RouteManager* rm,const RouteConfig& rc,bool traceit)
{
  static smsc::logger::Logger* log=smsc::logger::Logger::getInstance("loadroutes");
  try
  {
    Route *route;
    for (RouteConfig::RouteIterator ri = rc.getRouteIterator();
         ri.fetchNext(route) == RouteConfig::success;)
    {
      char * dest_key;
      char* src_key;
      Source src;
      Destination dest;
      RouteInfo* rinfo;
      RoutePoint rp;
      int pairsCount=0;
      FixedLengthString<64> srcSubj,dstSubj;
      for (DestinationHash::Iterator dest_it = route->getDestinations().getIterator();
           dest_it.Next(dest_key, dest);)
      {
        for (SourceHash::Iterator src_it = route->getSources().getIterator();
             src_it.Next(src_key, src);)
        {
          rinfo=new RouteInfo;
          rinfo->smeSystemId = dest.getSmeIdString();//dest.smeId;
          rinfo->srcSmeSystemId = route->getSrcSmeSystemId();
          //              __trace2__("sme sysid: %s",rinfo.smeSystemId.c_str());
          rinfo->billing = route->getBilling();
          //rinfo.paid =
          rinfo->archived=route->isArchiving();
          rinfo->trafMode= route->getTrafficMode();
          rinfo->routeId=route->getId();
          rinfo->serviceId=route->getServiceId();
          rinfo->priority=route->getPriority();
          rinfo->suppressDeliveryReports=route->isSuppressDeliveryReports();
          rinfo->hide=route->isHide();
          rinfo->replyPath=route->getReplyPath();
          rinfo->deliveryMode = route->getDeliveryMode();
          rinfo->forwardTo = route->getForwardTo();
          rinfo->forceDelivery=route->isForceDelivery();
          rinfo->aclId=route->getAclId();
          rinfo->allowBlocked=route->isAllowBlocked();
          rinfo->providerId=(int32_t)route->getProviderId();
//          rinfo->billingId=route->getBillingRuleId();
          rinfo->categoryId=(int32_t)route->getCategoryId();
          rinfo->transit=route->isTransit();
          rinfo->backupSme=route->getBackupSme();

          pairsCount=0;
          // masks
          if(dest.isSubject())
          {
            rinfo->dstSubj="subj:"+dest.getIdString();
          }
          if(src.isSubject())
          {
            rinfo->srcSubj="subj:"+src.getIdString();
          }
          const MaskVector& dest_masks = dest.getMasks();
          for (MaskVector::const_iterator dest_mask_it = dest_masks.begin();
               dest_mask_it != dest_masks.end();
               ++dest_mask_it)
          {
            try{
              makeAddress(rp.dest,*dest_mask_it);
            }catch(std::exception& e)
            {
              __warning2__("[route skiped] %s -> %s: %s",
                           route->getSrcSmeSystemId().c_str(), dest.getSmeIdString().c_str(), e.what());
              continue;
            }
            const MaskVector& src_masks = src.getMasks();
            for(MaskVector::const_iterator src_mask_it = src_masks.begin();
                src_mask_it != src_masks.end();
                ++src_mask_it)
            {
              try{
                makeAddress(rp.source,*src_mask_it);
                rm->addRoute(rinfo,rp);
                pairsCount++;
              }
              catch(exception& e)
              {
                __warning2__("[route skiped] %s -> %s: %s",
                             rinfo->srcSmeSystemId.c_str(), rinfo->smeSystemId.c_str(), e.what());
              }
            }
          }
          smsc_log_info(log,"Route %s:%d pairs",rinfo->routeId.c_str(),pairsCount);
        }
      }
    }
    rm->commit(traceit);
  }
  catch(...)
  {
    rm->cancel();
    throw;
  }
}

} // namespace system
} // namespace smsc

#if defined LOAD_ROUTES_TEST

int main(int argc, char** argv)
{
  try
  {
    RouteManager rm;
    SmeManager smeman;
    {
      SmeInfo si;
      si.typeOfNumber=1;
      si.numberingPlan=1;
      si.interfaceVersion=1;
      si.systemType=string("");
      si.password=string("");
      si.systemId=string("sme1");
      si.disabled=false;
      smeman.addSme(si);
    }
    rm.assign(&smeman);
    smsc::system::loadRoutes(&rm,"./routes.xml");
  }
  catch(exception& e)
  {
    cerr << "exception :" << e.what() << endl;
  }
  catch(...)
  {
    cerr << "unknown exeception " << endl;
  }
  return 0;
}

#endif
