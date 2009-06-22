#ifndef SCAG_BILL_EWALLET_PROTO_CONFIG_H
#define SCAG_BILL_EWALLET_PROTO_CONFIG_H

#include "scag/util/Time.h"
#include <string>

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

class Config
{
public:
    Config() :
    ioTimeout_(200),
    processTimeout_(1000),
    connectTimeout_(3000),
    inactivityTimeout_(20000),
    socketQueueSizeLimit_(100),
    connectionsCount_(1),
    maxWriterSocketCount_(5),
    maxReaderSocketCount_(5),
    readersCount_(1),
    writersCount_(1),
    host_("127.0.0.1"),
    port_(23881),
    enabled_(true) {}

    util::msectime_type getIOTimeout() const { return ioTimeout_; }
    util::msectime_type getProcessTimeout() const { return processTimeout_; }
    util::msectime_type getConnectTimeout() const { return connectTimeout_; }
    util::msectime_type getInactivityTimeout() const { return inactivityTimeout_; }

    size_t getSocketQueueSizeLimit() const { return socketQueueSizeLimit_; }

    size_t getConnectionsCount() const { return connectionsCount_; }


    size_t getMaxWriterSocketCount() const { return maxWriterSocketCount_; }
    size_t getMaxReaderSocketCount() const { return maxReaderSocketCount_; }
    size_t getReadersCount() const { return readersCount_; }
    size_t getWritersCount() const { return writersCount_; }

    const std::string& getHost() const { return host_; }
    
    const short getPort() const { return port_; }

    bool isEnabled() const { return enabled_; }
    void setEnabled( bool e ) { enabled_ = e; }

    std::string toString() const {
        std::string res;
        res.reserve(200);
        char buf[100];
        res.append( enabled_ ? "enabled" : "disabled");
        snprintf(buf,sizeof(buf)," tmo(io)=%u",unsigned(ioTimeout_));
        res.append(buf);
        snprintf(buf,sizeof(buf)," queueSize=%u", unsigned(socketQueueSizeLimit_));
        return res;
    }

private:
    util::msectime_type ioTimeout_;
    util::msectime_type processTimeout_;
    util::msectime_type connectTimeout_;
    util::msectime_type inactivityTimeout_;
    size_t socketQueueSizeLimit_;
    size_t connectionsCount_;
    size_t maxWriterSocketCount_;
    size_t maxReaderSocketCount_;
    size_t readersCount_;
    size_t writersCount_;
    std::string host_;
    short       port_;
    bool   enabled_;
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PROTO_CONFIG_H */
