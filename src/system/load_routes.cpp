
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
  int npi,tni;
  char value[21];
  memset(value,0,21);
  int scaned = sscanf(mask.c_str(),".%d.%d.%20s",
        &tni,
        &npi,
        value);
   if ( scaned != 3 )
   {
     scaned = sscanf(mask.c_str(),"+%[0123456789?]20s",value);
     if ( scaned )
     {
       npi = 1;//ISDN
       tni = 1;//INTERNATIONAL
     }
     else
     {
       scaned = sscanf(mask.c_str(),"%[0123456789?]20s",value);
       if ( !scaned )
       {
         __trace2__("incorrect format of 'addr = \"%20s\"'",  mask.c_str());
         throw runtime_error("incorrect format");
       }
       else
       {
         npi = 1;//ISDN
         tni = 2;//NATIONAL
       }
     }
   }
  addr.setValue(strlen(value),value);
  addr.setTypeOfNumber((uint8_t)tni);
  addr.setNumberingPlan((uint8_t)npi);
}

void loadRoutes(RouteManager* rm,RouteConfig& rc)
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
          const MaskVector& dest_masks = dest.getMasks();
          for (MaskVector::const_iterator dest_mask_it = dest_masks.begin();
               dest_mask_it != dest_masks.end();
               ++dest_mask_it)
          {
            makeAddress(rinfo.dest,*dest_mask_it);
            const MaskVector& src_masks = src.getMasks();
            for(MaskVector::const_iterator src_mask_it = src_masks.begin();
                src_mask_it != src_masks.end();
                ++src_mask_it)
            {
              makeAddress(rinfo.source,*src_mask_it);
              rinfo.smeSystemId = dest.getSmeIdString();//dest.smeId;
              __trace2__("sme sysid: %s",rinfo.smeSystemId.c_str());
              rinfo.billing = route->isBilling();
              //rinfo.paid =
              rinfo.archived=route->isArchiving();
              rinfo.enabling = route->isEnabling();
              rinfo.routeId=route->getId();
              rinfo.serviceId=route->getServiceId();
              rinfo.priority=route->getPriority();
              __trace2__("dest mask: %s",dest_mask_it->c_str());
              __trace2__("src mask: %s",src_mask_it->c_str());
              print(rinfo);
              rm->addRoute(rinfo);
            }
          }
        }
      }
    }
    rm->commit();
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
