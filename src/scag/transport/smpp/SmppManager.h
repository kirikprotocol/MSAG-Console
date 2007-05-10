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

namespace scag{
namespace transport{
namespace smpp{

namespace buf=smsc::core::buffers;
namespace thr=smsc::core::threads;
namespace sync=smsc::core::synchronization;
using namespace scag::config;
using namespace scag::lcm;

class SmppManager :
  public SmppManagerAdmin,
  public SmppChannelRegistrator,
  public SmppCommandQueue,
  public SmppRouter
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

    static SmppManager& Instance();
    static void Init(const char* cfgFile);
    static void shutdown();
};

}//smpp
}//transport
}//scag


#endif
