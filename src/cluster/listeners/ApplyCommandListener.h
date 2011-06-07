#ifndef ApplyCommandListner_dot_h
#define ApplyCommandListner_dot_h

#include "cluster/Interconnect.h"
#include "smeman/smeman.h"
#include "router/route_manager.h"
#include "core/synchronization/Mutex.hpp"
//#include "alias/AliasMan.hpp"
#include "admin/smsc_service/SmscComponent.h"
#include "system/smsc.hpp"

namespace smsc {
namespace cluster {

using smsc::system::Reffer;
using smsc::system::RefferGuard;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::router::RouteManager;
using smsc::alias::AliasManager;
using smsc::system::Smsc;

class ApplyCommandListener : public CommandListener
{
public:
    ApplyCommandListener(const smsc::system::SmscConfigs *configs_, /*smsc::smeman::SmeManager *smeman_, */smsc::system::Smsc * app_);
protected:
    void applyRoutes();
    void applyAliases();
    void applyReschedule();
    void applyLocalResource();
    void applyConfig();
    const smsc::system::SmscConfigs *configs;
    //smsc::smeman::SmeManager *smeman;

    /*void ResetRouteManager(RouteManager* manager)
    {
        MutexGuard g(routerSwitchMutex);
        if ( router_ ) router_->Release();
        router_ = new Reffer<RouteManager>(manager);
    }

    void ResetAliases(AliasManager* manager)
    {
        MutexGuard g(aliasesSwitchMutex);
        if ( aliaser_ ) aliaser_->Release();
        aliaser_ = new Reffer<AliasManager>(manager);
    }*/

    /*Mutex aliasesSwitchMutex;
    Mutex routerSwitchMutex;
    Reffer<RouteManager>* router_;
    Reffer<AliasManager>* aliaser_;*/
    Smsc * app;

public:
  virtual void handle(const Command& command);
};

}
}

#endif
