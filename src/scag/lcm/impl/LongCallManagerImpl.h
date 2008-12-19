#ifndef SCAG_LCM_IMPL_LONGCALLMANAGERIMPL_H
#define SCAG_LCM_IMPL_LONGCALLMANAGERIMPL_H

#include "logger/Logger.h"
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "scag/config/base/ConfigListener2.h"
#include "scag/re/base/LongCallContextBase.h"
#include "scag/lcm/base/LongCallManager2.h"

namespace scag2 {
namespace lcm {

class LongCallManagerImpl: public LongCallManager, public config::ConfigListener
{
public:
    LongCallManagerImpl(): ConfigListener(config::LONGCALLMAN_CFG), stopped(true) {}
    virtual ~LongCallManagerImpl() {}
    virtual void shutdown();
    
    /// invoked from main
    void init(uint32_t _maxThreads);

    /// interface for tasks
    LongCallContextBase* getContext();
    bool call(LongCallContextBase* context);

protected:
    virtual void configChanged();

protected:
    bool stopped;
    smsc::logger::Logger* logger;
    EventMonitor mtx;
    uint32_t maxThreads;
    smsc::core::threads::ThreadPool pool;
    LongCallContextBase *headContext, *tailContext;
    
};

} // namespace lcm
} // namespace scag2

#endif /* !SCAG_LCM_IMPL_LONGCALLMANAGERIMPL_H */
