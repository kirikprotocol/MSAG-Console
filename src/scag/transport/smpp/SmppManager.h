#ifndef __SCAG_TRANSPORT_SMPP_SMPPMANAGER_H__
#define __SCAG_TRANSPORT_SMPP_SMPPMANAGER_H__

#include "SmppManagerAdmin.h"
#include "logger/Logger.h"
#include "core/buffers/Hash.hpp"
#include "SmppEntity.h"
#include "SmppChannelRegistrator.h"
#include "SmppCommandQueue.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "SmppSocketManager.h"
#include "core/buffers/CyclicQueue.hpp"
#include "core/buffers/RefPtr.hpp"
#include "router/route_manager.h"
#include "SmppRouter.h"
#include "core/threads/ThreadPool.hpp"
#include "scag/config/ConfigListener.h"
#include "scag/lcm/LongCallManager.h"
#include "scag/util/Reffer.h"

namespace scag{
namespace transport{
namespace smpp{

namespace buf=smsc::core::buffers;
namespace thr=smsc::core::threads;
namespace sync=smsc::core::synchronization;
using namespace scag::config;
using namespace scag::lcm;
using namespace scag::util;
using namespace scag::transport::smpp::router;

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

    virtual void  sendReceipt(Address& from, Address& to, int state, const char* msgId, const char* dst_sme_id) = 0;
    virtual void pushCommand(SmppCommand& cmd) = 0;
    virtual void continueExecution(LongCallContext* lcmCtx, bool dropped) = 0;
    virtual bool makeLongCall(SmppCommand& cx, SessionPtr& session) = 0;

    virtual void reloadTestRoutes(const RouteConfig& rcfg) = 0;
    virtual RefferGuard<RouteManager> getTestRouterInstance() = 0;
    virtual void ResetTestRouteManager(RouteManager* manager) = 0;
    virtual void getQueueLen(uint32_t& reqQueueLen, uint32_t& respQueueLen, uint32_t& lcmQueueLen) = 0;
    static SmppManager& Instance();
    static void Init(const char* cfgFile);
    static void shutdown();
};

}//smpp
}//transport
}//scag


#endif
