#ifndef _SCAG_TRANSPORT_SMPP_BASE_SMPPMANAGER2_H
#define _SCAG_TRANSPORT_SMPP_BASE_SMPPMANAGER2_H

#include "SmppChannelRegistrator2.h"
#include "SmppCommandQueue2.h"
#include "SmppEntity2.h"
#include "SmppManagerAdmin2.h"
#include "SmppRouter2.h"
// #include "SmppSocketManager2.h"
// #include "core/buffers/CyclicQueue.hpp"
// #include "core/buffers/Hash.hpp"
// #include "core/buffers/RefPtr.hpp"
// #include "core/synchronization/EventMonitor.hpp"
// #include "core/synchronization/Mutex.hpp"
// #include "core/threads/ThreadPool.hpp"
// #include "logger/Logger.h"
#include "scag/transport/smpp/router/route_manager.h"
// #include "scag/config/base/ConfigListener2.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/re/base/LongCallContext.h"
#include "scag/util/Reffer.h"

namespace scag2 {

namespace sessions {
    class ActiveSession;
}

namespace transport {
namespace smpp {

using namespace lcm;
using namespace config;

class SmppManager :
public SmppManagerAdmin,
public SmppChannelRegistrator,
public SmppCommandQueue,
public SmppRouter,
public LongCallInitiator
{
public:
    static SmppManager& Instance();

    // should be invoked once
    // static void setInstance( SmppManager* mgr );
    virtual ~SmppManager();

    virtual void LoadRoutes(const char* cfgFile) = 0;
    virtual void ReloadRoutes() = 0;

    virtual void  sendReceipt( Address& from,
                               Address& to,
                               int state, 
                               const char* msgId,
                               const char* dst_sme_id,
                               uint32_t netErrCode ) = 0;

    /// accept partially processed commands from LCM
    virtual void pushCommand( SmppCommand* cmd ) = 0;

    /// This method accepts next command from session queue, taking their ownership.
    /// See SCAGCommandQueue for description of the second parameter.
    /// \return queue size if success, -1 for failure (when queue is stopped),
    /// but the command is taken anyway!
    virtual unsigned pushSessionCommand( SmppCommand* cmd,
                                         int action = SCAGCommandQueue::PUSH ) = 0;

    /// if success cx is cleared and session is left locked, true is returned.
    virtual bool makeLongCall( std::auto_ptr<SmppCommand>& cx,
                               sessions::ActiveSession& session ) = 0;

    // virtual void continueExecution( LongCallContext* lcmCtx, bool dropped ) = 0;

    virtual void reloadTestRoutes(const RouteConfig& rcfg) = 0;
    virtual util::RefferGuard<router::RouteManager> getTestRouterInstance() = 0;
    virtual void ResetTestRouteManager(router::RouteManager* manager) = 0;
    virtual void getQueueLen( uint32_t& reqQueueLen,
                              uint32_t& respQueueLen,
                              uint32_t& lcmQueueLen) = 0;

    // static void Init(const char* cfgFile);
    virtual void shutdown() = 0;

protected:
    SmppManager();

private:
    SmppManager( const SmppManager& );
    SmppManager& operator = ( const SmppManager& );
};

}//smpp
}//transport
}//scag


#endif
