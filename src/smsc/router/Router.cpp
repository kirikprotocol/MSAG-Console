/*
 * Router.cpp
 *
 *  Created on: Jul 30, 2010
 *      Author: skv
 */

#include "smsc/router/Router.hpp"

namespace smsc{
namespace router{

Router* Router::instance=0;



void Router::Load(smsc::smeman::SmeTable* st,const smsc::config::route::RouteConfig& rc)
{
  using namespace smsc::config::route;
  using smsc::sms::Address;
  Route* route;
  int count=0,dstCount=0;

  SmeRoutingInfo* tempRoutingInfo[smsc::smeman::MAX_SME_PROXIES]={0,};
  for (RouteConfig::RouteIterator ri = rc.getRouteIterator();
       ri.fetchNext(route) == RouteConfig::success;)
  {
    try{
      char * dest_key;
      char* src_key;
      Source src;
      Destination dest;
      RouteInfo* rinfo=new RouteInfo;
      count++;

      smsc_log_debug(log,"processing route '%s'",route->getId());

      rinfo->srcSmeSystemId = route->getSrcSmeSystemId();
      int smeIndex=st->lookup(rinfo->srcSmeSystemId);
      if(smeIndex<0 || smeIndex>=smsc::smeman::MAX_SME_PROXIES)
      {
        smsc_log_debug(log,"Invalid smeIndex for sme '%s' (route %s)",rinfo->srcSmeSystemId.c_str(),route->getId());
        continue;
      }
      rinfo->billing = route->getBilling();
      rinfo->archived=route->isArchiving();
      rinfo->trafMode= route->getTrafMode();
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
      rinfo->categoryId=(int32_t)route->getCategoryId();
      rinfo->transit=route->isTransit();

      SmeRoutingInfo* sri=tempRoutingInfo[smeIndex];

      if(sri==0)
      {
        sri=new SmeRoutingInfo;
        tempRoutingInfo[smeIndex]=sri;
        sri->srcSystemId=rinfo->srcSmeSystemId;
      }

      DestRoute* dstRoute=0;

      for (SourceHash::Iterator src_it = route->getSources().getIterator();src_it.Next(src_key, src);)
      {
        const MaskVector& src_masks = src.getMasks();
        for(MaskVector::const_iterator src_mask_it = src_masks.begin();src_mask_it != src_masks.end();++src_mask_it)
        {
          Address sourceAddr=src_mask_it->c_str();
          if(!dstRoute)
          {
            if(!sri->srcRoute.Find(sourceAddr.toString().c_str(),dstRoute))
            {
              dstRoute=new DestRoute;
              smsc_log_debug(log,"new dest group %p",dstRoute);
            }
          }
          sri->srcRoute.Insert(sourceAddr.toString().c_str(),dstRoute);
          smsc_log_debug(log,"%s->%p",sourceAddr.toString().c_str(),dstRoute);
        }
      }
      for (DestinationHash::Iterator dest_it = route->getDestinations().getIterator();dest_it.Next(dest_key, dest);)
      {
        if(rinfo->destSmeSystemId.empty())
        {
          rinfo->destSmeSystemId = dest.getSmeIdString();//dest.smeId;
          rinfo->destSmeIndex=st->lookup(rinfo->destSmeSystemId);
        }else
          if(rinfo->destSmeSystemId != dest.getSmeIdString().c_str())
          {
            rinfo=new RouteInfo(*rinfo);
            rinfo->destSmeSystemId = dest.getSmeIdString();//dest.smeId;
            rinfo->destSmeIndex=st->lookup(rinfo->destSmeSystemId);
          }

        const MaskVector& dest_masks = dest.getMasks();
        for (MaskVector::const_iterator dest_mask_it = dest_masks.begin();dest_mask_it != dest_masks.end();++dest_mask_it)
        {
          Address destAddr=dest_mask_it->c_str();
          smsc_log_debug(log,"add route %s->%s(%s) to %p",rinfo->srcSmeSystemId.c_str(),rinfo->destSmeSystemId.c_str(),destAddr.toString().c_str(),dstRoute);
          dstRoute->Insert(destAddr.toString().c_str(),rinfo);
          dstCount++;
        }
      }
    }catch(std::exception& e)
    {
      smsc_log_warn(log,"route '%s' skipped:%s",route->getId(),e.what());
    }
  }
  for(size_t i=0;i<smsc::smeman::MAX_SME_PROXIES;i++)
  {
    smeRouting[i]=tempRoutingInfo[i];
  }
  smsc_log_info(log,"%d routes loaded, total %d destinations",count,dstCount);
}

bool Router::lookup(smsc::smeman::SmeIndex srcSmeIdx,const smsc::sms::Address& srcAddr,const smsc::sms::Address& dstAddr,RoutingResult& rr)
{
  rr.found=false;
  if(srcSmeIdx<0 || srcSmeIdx>=smsc::smeman::MAX_SME_PROXIES)
  {
    smsc_log_error(log,"Invalid srcSmeIdx=%d",srcSmeIdx);
    return false;
  }
  SmeRouterRef sr=smeRouting[srcSmeIdx];
  if(!sr.Get())
  {
    smsc_log_debug(log,"Router for srcSmeIdx=%d not found",srcSmeIdx);
    return false;
  }
  DestRoute* dr=0;
  if(!sr->srcRoute.Find(srcAddr.toString().c_str(),dr))
  {
    smsc_log_debug(log,"Route for srcSme=%s, srcAddr=%s not found",sr->srcSystemId.c_str(),srcAddr.toString().c_str());
    return false;
  }
  RouteInfo* res;
  if(!dr->Find(dstAddr.toString().c_str(),res))
  {
    smsc_log_debug(log,"Route for srcSme=%s, srcAddr=%s, dstAddr=%s not found (%p)",sr->srcSystemId.c_str(),srcAddr.toString().c_str(),dstAddr.toString().c_str(),dr);
    return false;
  }
  smsc_log_debug(log,"Route for srcSme=%s, srcAddr=%s, dstAddr=%s found '%s'(%p)",sr->srcSystemId.c_str(),srcAddr.toString().c_str(),dstAddr.toString().c_str(),res->routeId.c_str(),dr);
  rr.found=true;
  rr.info=*res;
  //rr.destSmeIdx=
  return res->trafMode!=tmNone;
}



}
}
