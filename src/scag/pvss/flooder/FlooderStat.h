#ifndef _SCAG_PVSS_FLOODER_FLOODERSTAT_H
#define _SCAG_PVSS_FLOODER_FLOODERSTAT_H

#include <memory>
#include <string>
#include "core/synchronization/EventMonitor.hpp"
#include "scag/pvss/api/core/client/Client.h"
#include "RequestGenerator.h"
#include "FlooderConfig.h"
#include "core/threads/ThreadPool.hpp"
#include "scag/util/WatchedThreadedTask.h"
#include "scag/pvss/api/core/Statistics.h"

namespace scag2 {
namespace pvss {
namespace flooder {

class FlooderStat : public core::client::Client::ResponseHandler
{
public:
    FlooderStat( const FlooderConfig& config,
                 core::client::Client& client );

    virtual ~FlooderStat();

    RequestGenerator& getGenerator() { return generator_; }

    core::client::Client& getClient() { return client_; }

    /// notify that response is got
    virtual void handleResponse(std::auto_ptr<Request> request, std::auto_ptr<Response> response);

    /// notify that request is not sent
    virtual void handleError(const PvssException& exc, std::auto_ptr<Request> request);

    /// notify that request is sent
    void requestCreated();

    /// adjust speed (wait until the speed is less or equal the one requested)
    void adjustSpeed();

    /// get accounting information (total)
    core::Statistics getTotals();
    /// get accounting info (last full filled chunk)
    core::Statistics getCurrent();

    /// wait until requested number of requests are produced and responses obtained (or timeouted)
    void waitUntilProcessed();

    void startup() throw (exceptions::IOException);

    /// shutdown
    void shutdown();

    bool isStopped() const { return stopped_; }

private:
    /// check time and reset statistics if needed
    void checkTime();

private:    
    static const util::msectime_type accumulationTime = 5000; // 5 seconds accumulation time

private:
    smsc::core::synchronization::EventMonitor              mon_;
    smsc::logger::Logger*                                  log_;
    unsigned                                               requestsPerSecond_;
    unsigned                                               requested_;
    util::msectime_type                                    doneTime_;
    util::msectime_type                                    timeout_;
    bool                                                   stopped_;
    core::Statistics                                       total_;
    core::Statistics                                       previous_;
    core::Statistics                                       last_;
    const FlooderConfig&                                   config_;
    RequestGenerator                                       generator_;
    core::client::Client&                                  client_;
    std::auto_ptr<smsc::core::threads::ThreadPool>         tp_;
    smsc::core::buffers::Array<util::WatchedThreadedTask*> tasks_;
};

} // namespace flooder
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_FLOODER_FLOODERSTAT_H */
