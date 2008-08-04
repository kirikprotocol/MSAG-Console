#ifndef _SCAG_TRANSPORT_SMPP_SMPPMANAGER2_H
#define _SCAG_TRANSPORT_SMPP_SMPPMANAGER2_H

#include "SmppChannelRegistrator2.h"
#include "SmppCommandQueue2.h"
#include "SmppEntity2.h"
#include "SmppManagerAdmin2.h"
#include "SmppRouter2.h"
#include "SmppSocketManager2.h"
#include "core/buffers/CyclicQueue.hpp"
#include "core/buffers/Hash.hpp"
#include "core/buffers/RefPtr.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"
#include "router/route_manager.h"
#include "scag/config/ConfigListener2.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/lcm/LongCallManager2.h"
#include "scag/util/Reffer.h"

namespace scag2 {

namespace sessions {
    class ActiveSession;
}

namespace transport {
namespace smpp {

namespace buf=smsc::core::buffers;
namespace thr=smsc::core::threads;
namespace sync=smsc::core::synchronization;

using namespace scag2::lcm;
using namespace scag2::config;
using namespace scag::util;

class SmppManager :
  public SmppManagerAdmin,
  public SmppChannelRegistrator,
  public SmppCommandQueue,
  public SmppRouter,
  public LongCallInitiator
{
    static bool  inited;
    static Mutex initLock;

public:
    SmppManager() {};
    virtual ~SmppManager() {};

    virtual void LoadRoutes(const char* cfgFile) = 0;
    virtual void ReloadRoutes() = 0;

    virtual void  sendReceipt(Address& from, Address& to, int state, const char* msgId, const char* dst_sme_id, uint32_t netErrCode) = 0;

    // taking ownership
    virtual void pushCommand(SmppCommand* cmd) = 0;

    // if success cx is cleared and session is leave locked.
    virtual bool makeLongCall( std::auto_ptr<SmppCommand>& cx,
                               ActiveSession& session ) = 0;
    virtual void continueExecution(LongCallContext* lcmCtx, bool dropped) = 0;

    virtual void reloadTestRoutes(const RouteConfig& rcfg) = 0;
    virtual RefferGuard<router::RouteManager> getTestRouterInstance() = 0;
    virtual void ResetTestRouteManager(router::RouteManager* manager) = 0;
    virtual void getQueueLen(uint32_t& reqQueueLen, uint32_t& respQueueLen, uint32_t& lcmQueueLen) = 0;
    static SmppManager& Instance();
    static void Init(const char* cfgFile);
    static void shutdown();
};

}//smpp
}//transport
}//scag


#endif
