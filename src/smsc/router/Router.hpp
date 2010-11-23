/*
 * Router.hpp
 *
 *  Created on: Jul 30, 2010
 *      Author: skv
 */

#ifndef __SMSC_ROUTER_ROUTER_HPP__
#define __SMSC_ROUTER_ROUTER_HPP__

#include "route_types.h"
#include "smsc/config/route/RouteConfig.h"
#include "core/buffers/IntHash.hpp"
#include "smsc/smeman/smetypes.h"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/RefPtr.hpp"
#include "core/buffers/XTree.hpp"
#include "smsc/smeman/smetable.h"
#include <set>
#include "logger/Logger.h"

namespace smsc{
namespace router{

namespace buf=smsc::core::buffers;
namespace sync=smsc::core::synchronization;

struct RoutingResult{
  bool found;
  RouteInfo info;
  smsc::smeman::SmeIndex destSmeIdx;
  smsc::smeman::SmeProxy* destProxy;
};

class Router
{
public:
  void Load(smsc::smeman::SmeTable* st,const smsc::config::route::RouteConfig& rc);

  static Router* getInstance()
  {
    return instance;
  }
  static void Init()
  {
    instance=new Router;
    instance->log=smsc::logger::Logger::getInstance("router");
  }
  bool lookup(smsc::smeman::SmeIndex srcSmeIdx,const smsc::sms::Address& srcAddr,const smsc::sms::Address& dstAddr,RoutingResult& rr);
protected:
  static Router* instance;
  smsc::logger::Logger* log;
  typedef buf::XTree<RouteInfo*> DestRoute;
  struct SmeRoutingInfo{
    struct Deleter{
      std::set<RouteInfo*> rinfo;
      std::set<DestRoute*> dstRoutes;
      void operator()(DestRoute* data)
      {
        data->ForEach(*this);
        dstRoutes.insert(data);
      }
      void operator()(RouteInfo* data)
      {
        rinfo.insert(data);
      }
      ~Deleter()
      {
        for(std::set<RouteInfo*>::iterator it=rinfo.begin(),end=rinfo.end();it!=end;++it)
        {
          delete *it;
        }
        for(std::set<DestRoute*>::iterator it=dstRoutes.begin(),end=dstRoutes.end();it!=end;++it)
        {
          delete *it;
        }
      }
    };
    ~SmeRoutingInfo()
    {
      Deleter d;
      srcRoute.ForEach(d);
    }
    buf::XTree<DestRoute*> srcRoute;
    smsc::smeman::SmeSystemId srcSystemId;
  };
  typedef buf::RefPtr<SmeRoutingInfo,sync::Mutex> SmeRouterRef;
  SmeRouterRef smeRouting[smsc::smeman::MAX_SME_PROXIES];
};

}
}

#endif /* ROUTER_HPP_ */
