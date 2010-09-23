#include "load_routes.h"
#include "sms/sms.h"

namespace scag{
namespace transport{
namespace smpp{
namespace router{

using smsc::sms::Address;
using namespace scag::config;

static inline void makeAddress(Address& addr,const string& mask)
{
  addr=Address(mask.c_str());
}

void loadRoutes(RouteManager* rm,const scag::config::RouteConfig& rc,bool traceit)
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
              // rinfo.archived=route->isArchiving();
              rinfo.transit = route->isTransit();
              rinfo.statistics = route->hasStatistics();
              rinfo.hideMessage = route->hideMessageBody();
              rinfo.routeId=route->getId();
              rinfo.serviceId=route->getServiceId();
              rinfo.enabled = true; // route->isEnabling();
              rinfo.slicing=route->getSlicingType();
              rinfo.slicingRespPolicy=route->getSlicingRespPolicy();
              try{
                rm->addRoute(rinfo);
              }
              catch(exception& e)
              {
                __warning2__("[route skiped] : %s",e.what());
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

}//namespace scag
}//namespace transport
}//namespace smpp
}//namespace router
