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

    ServerConfig() : Config(), queueSizeLimit(DEF_QUEUE_SIZE_LIMIT) {}

    int getQueueSizeLimit() const {
        return queueSizeLimit;
    }

    void setQueueSizeLimit( int queueSizeLimit ) throw(ConfigException)
    {
        if (queueSizeLimit <= MIN_QUEUE_SIZE_LIMIT || queueSizeLimit > MAX_QUEUE_SIZE_LIMIT)
            throw ConfigException("Pending requests queue size limit can't be less than %d and more then %d",
                                  MIN_QUEUE_SIZE_LIMIT, MAX_QUEUE_SIZE_LIMIT);
        this->queueSizeLimit = queueSizeLimit;
    }

private:
    int queueSizeLimit;

};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_SERVERCONFIG_H */
