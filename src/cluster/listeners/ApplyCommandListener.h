#ifndef ApplyCommandListner_dot_h
#define ApplyCommandListner_dot_h

#include "cluster/Interconnect.h"
#include "system/smsc.hpp"
#include "smeman/smeman.h"
#include "router/route_manager.h"
#include "core/synchronization/Mutex.hpp"
#include "alias/aliasman.h"
#include "admin/smsc_service/SmscComponent.h"

namespace smsc { 
namespace cluster {

using smsc::system::Reffer;
using smsc::system::RefferGuard;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::router::RouteManager;
using smsc::alias::AliasManager;

class ApplyCommandListener : public CommandListener
{
public:
    ApplyCommandListener(smsc::system::SmscConfigs &configs_, smsc::smeman::SmeManager *smeman_);
protected:
	void applyRoutes();
    void applyAliases();
    void applyReschedule();
    smsc::system::SmscConfigs &configs;
    smsc::smeman::SmeManager *smeman;

    void ResetRouteManager(RouteManager* manager)
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
    }

    Mutex aliasesSwitchMutex;
    Mutex routerSwitchMutex;
    Reffer<RouteManager>* router_;
    Reffer<AliasManager>* aliaser_;

public:
	virtual void handle(const Command& command);
};

}
}

#endif
