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
    host_("phoenix"),
    port_(7990),
    enabled_(true),
    loopback_(false) {}

    util::msectime_type getIOTimeout() const { return ioTimeout_; }
    void setIOTimeout( util::msectime_type tmo ) { ioTimeout_ = tmo; }
    util::msectime_type getProcessTimeout() const { return processTimeout_; }
    void setProcessTimeout( util::msectime_type tmo ) { processTimeout_ = tmo; }
    util::msectime_type getConnectTimeout() const { return connectTimeout_; }
    void setConnectTimeout( util::msectime_type tmo ) { connectTimeout_ = tmo; }
    util::msectime_type getInactivityTimeout() const { return inactivityTimeout_; }
    // void setConnectTimeout( util::msectime_type tmo ) { connectTimeout_ = tmo; }

    size_t getSocketQueueSizeLimit() const { return socketQueueSizeLimit_; }
    void setSocketQueueSizeLimit( size_t limit ) { socketQueueSizeLimit_ = limit; }

    size_t getConnectionsCount() const { return connectionsCount_; }
    void setConnectionsCount( size_t count ) { connectionsCount_ = count; }

    size_t getMaxWriterSocketCount() const { return maxWriterSocketCount_; }
    void setMaxWriterSocketCount( size_t count ) { maxWriterSocketCount_ = count; }
    size_t getMaxReaderSocketCount() const { return maxReaderSocketCount_; }
    void setMaxReaderSocketCount( size_t count ) { maxReaderSocketCount_ = count; }
    size_t getReadersCount() const { return readersCount_; }
    void setReadersCount( size_t count ) { readersCount_ = count; }
    size_t getWritersCount() const { return writersCount_; }
    void setWritersCount( size_t count ) { writersCount_ = count; }

    const std::string& getHost() const { return host_; }
    void setHost( const std::string& host ) { host_ = host; }
    
    const short getPort() const { return port_; }
    void setPort( short port ) { port_ = port; }

    bool isEnabled() const { return enabled_; }
    void setEnabled( bool e ) { enabled_ = e; }

    /// NOTE: if true then network connections will be replaced by a loopback wrapper.
    bool isLoopback() const { return loopback_; }
    void setLoopback( bool e ) { loopback_ = e; }

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
    bool   loopback_;
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PROTO_CONFIG_H */
