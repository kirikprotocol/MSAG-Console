#include <string>
#include <stdexcept>

#if 0
#include "smsc/config/route/RouteConfig.h"
#include "smsc/router/route_manager.h"
#include "smsc/smeman/smeman.h"
#include "sms/sms.h"

//#define LOAD_ROUTES_TEST

using namespace smsc::config::route;
using namespace smsc::sms;
using namespace std;
using namespace smsc::router;
using namespace smsc::smeman;

namespace smsc {
namespace common {

static inline void print(RouteInfo& info,const char* ppp= "")
{
   __trace2__("%s={SRC::%.20s(%d),n:%d,t:%d}{DST::%.20s(%d),n:%d,t:%d}",
             ppp,
             info.source.value,
             info.source.length,
             info.source.plan,
             info.source.type,
             info.dest.value,
             info.dest.length,
             info.dest.plan,
             info.dest.type
            );
}

static inline void makeAddress(Address& addr,const string& mask)
{
  addr=Address(mask.c_str());
}

void loadRoutes(RouteManager* rm,const RouteConfig& rc,bool traceit)
{
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
      RouteInfo rinfo;
      for (DestinationHash::Iterator dest_it = route->getDestinations().getIterator();
           dest_it.Next(dest_key, dest);)
      {
        for (SourceHash::Iterator src_it = route->getSources().getIterator();
             src_it.Next(src_key, src);)
        {
          // masks
          if(dest.isSubject())
          {
            rinfo.dstSubj="subj:"+dest.getIdString();
          }
          const MaskVector& dest_masks = dest.getMasks();
          for (MaskVector::const_iterator dest_mask_it = dest_masks.begin();
               dest_mask_it != dest_masks.end();
               ++dest_mask_it)
          {
            makeAddress(rinfo.dest,*dest_mask_it);
            if(!dest.isSubject())
            {
              rinfo.dstSubj="mask:"+*dest_mask_it;
            }
            const MaskVector& src_masks = src.getMasks();
            if(src.isSubject())
            {
              rinfo.srcSubj="subj:"+src.getIdString();
            }
            for(MaskVector::const_iterator src_mask_it = src_masks.begin();
                src_mask_it != src_masks.end();
                ++src_mask_it)
            {
              if(!src.isSubject())
              {
                rinfo.srcSubj="mask:"+*src_mask_it;
              }
              makeAddress(rinfo.source,*src_mask_it);
              rinfo.smeSystemId = dest.getSmeIdString();//dest.smeId;
              rinfo.srcSmeSystemId = route->getSrcSmeSystemId();
//              __trace2__("sme sysid: %s",rinfo.smeSystemId.c_str());
              rinfo.billing = route->isBilling();
              //rinfo.paid =
              rinfo.archived=route->isArchiving();
              rinfo.enabled = route->isEnabling();
              rinfo.routeId=route->getId();
              rinfo.serviceId=route->getServiceId();
              rinfo.priority=route->getPriority();
              rinfo.suppressDeliveryReports=route->isSuppressDeliveryReports();
              rinfo.hide=route->isHide();
              rinfo.replyPath=route->getReplyPath();
              rinfo.deliveryMode = route->getDeliveryMode();
              rinfo.forwardTo = route->getForwardTo();
              //rinfo.trafRules=TrafficRules(route->getTrafRules());
              rinfo.forceDelivery=route->isForceDelivery();
              rinfo.aclId=route->getAclId();
              rinfo.allowBlocked=route->isAllowBlocked();
              rinfo.providerId=(int32_t)route->getProviderId();
              //rinfo.billingId=route->getBillingRuleId();
              rinfo.categoryId=(int32_t)route->getCategoryId();
              rinfo.transit=route->isTransit();
//              __trace2__("dest mask: %s",dest_mask_it->c_str());
//              __trace2__("src mask: %s",src_mask_it->c_str());
//              print(rinfo);
              try{
                rm->addRoute(rinfo);
              }
              catch(exception& e)
              {
                __warning2__("[route skiped] %s -> %s: %s",
                             rinfo.srcSmeSystemId.c_str(), rinfo.smeSystemId.c_str(), e.what());
              }
            }
          }
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

#endif
