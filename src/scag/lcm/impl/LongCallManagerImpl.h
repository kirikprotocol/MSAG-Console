#ifndef SCAG_LCM_IMPL_LONGCALLMANAGERIMPL_H
#define SCAG_LCM_IMPL_LONGCALLMANAGERIMPL_H

#include "logger/Logger.h"
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "scag/config/base/ConfigListener2.h"
#include "scag/re/base/LongCallContextBase.h"
#include "scag/lcm/base/LongCallManager2.h"
#include "scag/pvss/base/PersCall.h"
#include "scag/pvss/api/core/client/Client.h"

namespace scag2 {
namespace lcm {

class LongCallManagerImpl: public LongCallManager, public config::ConfigListener, public pvss::PersCallInitiator
{
public:
    LongCallManagerImpl( pvss::core::client::Client* clnt ) :
    ConfigListener(config::LONGCALLMAN_CFG), pvssClient_(clnt), stopped(true) {}

    virtual ~LongCallManagerImpl() {
        delete pvssClient_;
    }
    virtual void shutdown();
    
    /// invoked from main
    void init(uint32_t _maxThreads);

    virtual pvss::core::client::Client& pvssClient() {
        return *pvssClient_;
    }

    /// interface for tasks
    LongCallContextBase* getContext();
    bool call(LongCallContextBase* context);

protected:
    virtual void configChanged();
    virtual void continuePersCall( pvss::PersCall* pc, bool drop );

protected:
    pvss::core::client::Client* pvssClient_; // owned

    bool stopped;
    smsc::logger::Logger* logger;
    smsc::core::synchronization::EventMonitor mtx;
    uint32_t maxThreads;
    smsc::core::threads::ThreadPool pool;
    LongCallContextBase *headContext, *tailContext;
    
};

} // namespace lcm
} // namespace scag2

#endif /* !SCAG_LCM_IMPL_LONGCALLMANAGERIMPL_H */
