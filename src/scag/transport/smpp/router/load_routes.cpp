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

void loadRoutes(RouteManager* rm,const scag::config::RouteConfig& rc,
                std::vector<std::string>* traceit )
{
  static smsc::logger::Logger* log=smsc::logger::Logger::getInstance("loadroutes");
  time_t start_stamp=time(0);
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
      int pairsCount=0;
      for (DestinationHash::Iterator dest_it = route->getDestinations().getIterator();
           dest_it.Next(dest_key, dest);)
      {
        for (SourceHash::Iterator src_it = route->getSources().getIterator();
             src_it.Next(src_key, src);)
        {
          pairsCount=0;
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
                pairsCount++;
              }
              catch(exception& e)
              {
                __warning2__("[route skiped] : %s",e.what());
              }
            }
          }
          smsc_log_info(log,"Route %s:%d pairs",rinfo.routeId.c_str(),pairsCount);
        }
      }
    }
    time_t load_stamp=time(0);
    rm->commit(traceit);
    smsc_log_info(log,"Routes load in %ld sec. Commit in %ld sec",load_stamp-start_stamp,time(0)-load_stamp);
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
