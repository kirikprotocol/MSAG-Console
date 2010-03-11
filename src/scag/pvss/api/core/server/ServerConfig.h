#ifndef _SCAG_PVSS_CORE_SERVER_SERVERCONFIG_H
#define _SCAG_PVSS_CORE_SERVER_SERVERCONFIG_H

#include "scag/pvss/api/core/Config.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

/// interface
class ServerConfig : public Config
{
public:
    const static int MIN_QUEUE_SIZE_LIMIT = 10;
    const static int DEF_QUEUE_SIZE_LIMIT = 100;
    const static int MAX_QUEUE_SIZE_LIMIT = 10000;
public:

    ServerConfig() :
    Config(),
    queueSizeLimit_(DEF_QUEUE_SIZE_LIMIT),
    timingInterval_(5000),
    timingSeriesSize_(5) {}

    int getQueueSizeLimit() const {
        return queueSizeLimit_;
    }

    void setQueueSizeLimit( int queueSizeLimit ) /* throw(ConfigException) */ 
    {
        if (queueSizeLimit <= MIN_QUEUE_SIZE_LIMIT || queueSizeLimit > MAX_QUEUE_SIZE_LIMIT)
            throw ConfigException("Pending requests queue size limit can't be less than %d and more then %d",
                                  MIN_QUEUE_SIZE_LIMIT, MAX_QUEUE_SIZE_LIMIT);
        this->queueSizeLimit_ = queueSizeLimit;
    }

    unsigned getTimingInterval() const { return timingInterval_; }
    unsigned getTimingSeriesSize() const { return timingSeriesSize_; }

    void setTimingInterval( unsigned ti ) {
        if (ti < 1000) throw ConfigException("timingInterval value=%u is invalid, must be >=1000",ti);
        timingInterval_ = ti;
    }

    void setTimingSeriesSize( unsigned tss ) {
        if (tss == 0 || tss > 100) throw ConfigException("timingSeriesSize value=%u is invalid, must be in [1,100]",tss);
        timingSeriesSize_ = tss;
    }

    std::string toString() const {
        std::string s(Config::toString());
        char buf[128];
        snprintf(buf,sizeof(buf)," queueSize=%d timingInterval=%u timingSeriesSize=%u",
                 queueSizeLimit_, timingInterval_, timingSeriesSize_ );
        return s + buf;
    }

private:
    int queueSizeLimit_;
    unsigned timingInterval_;
    unsigned timingSeriesSize_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_SERVERCONFIG_H */
