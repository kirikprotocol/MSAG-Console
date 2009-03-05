#ifndef _SCAG_PVSS_CORE_CONFIG_H
#define _SCAG_PVSS_CORE_CONFIG_H

#include "ConfigException.h"

namespace scag2 {
namespace pvss {
namespace core {

class Config
{
public:
    static const int MIN_CHANNEL_QUEUE_SIZE = 1;
    static const int DEF_CHANNEL_QUEUE_SIZE = 100;
    static const int MAX_CHANNEL_QUEUE_SIZE = 10000;

    static const int MIN_PACKET_SIZE = 4;        // for length
    static const int MAX_PACKET_SIZE = 1048576;  // 1Mb

    static const int MIN_IO_TIMEOUT = 10;    // 10  msec
    static const int DEF_IO_TIMEOUT = 100;   // 100 msec
    static const int MAX_IO_TIMEOUT = 60000; // 1 min

    static const long MIN_INACTIVITY_TIME     = 1000;                      // 1 sec
    static const long MAX_INACTIVITY_TIME     = MIN_INACTIVITY_TIME *3600; // 1 hour
    static const long DEFAULT_INACTIVITY_TIME = MIN_INACTIVITY_TIME *60;   // 1 min

    static const int MIN_CONNECT_TIMEOUT = 100;      // 100 ms
    static const int DEF_CONNECT_TIMEOUT = 60000;    // 1 min
    static const int MAX_CONNECT_TIMEOUT = 300000;   // 5 min

    static const int MIN_PROCESS_TIMEOUT = 1;        // 1 msec
    static const int DEF_PROCESS_TIMEOUT = 10;       // 10 msec
    static const int MAX_PROCESS_TIMEOUT = 1000;     // 1 sec

public:
    int getChannelQueueSizeLimit() const {
        return channelQueueSizeLimit_;
    }
    void setChannelQueueSizeLimit(int channelQueueSizeLimit) throw(ConfigException) {
        if (channelQueueSizeLimit < MIN_CHANNEL_QUEUE_SIZE || channelQueueSizeLimit > MAX_CHANNEL_QUEUE_SIZE)
            throw ConfigException( "Channel queue size limit can't be less than %d and more than %d",
                                   MIN_CHANNEL_QUEUE_SIZE, MAX_CHANNEL_QUEUE_SIZE );
        this->channelQueueSizeLimit_ = channelQueueSizeLimit;
    }

    int getPacketSizeLimit() const {
        return packetSizeLimit_;
    }
    void setPacketSizeLimit(int packetSizeLimit) throw(ConfigException) {
        if (packetSizeLimit < MIN_PACKET_SIZE || packetSizeLimit > MAX_PACKET_SIZE)
            throw ConfigException("PVAP packet size limit can't be less than %d and more than %d",
                                  MIN_PACKET_SIZE, MAX_PACKET_SIZE);
        this->packetSizeLimit_ = packetSizeLimit;
    }

    const std::string& getHost() const {
        return host_;
    }
    void setHost( const std::string& host) throw(ConfigException) {
        if ( host.empty() ) throw ConfigException("Host name should be provided");
        this->host_ = host;
    }

    short getPort() const {
        return port_;
    }
    void setPort(short port) {
        this->port_ = port;
    }

    int getIOTimeout() const {
        return ioTimeout_;
    }
    void setIOTimeout(int ioTimeout) throw(ConfigException) {
        if (ioTimeout < MIN_IO_TIMEOUT || ioTimeout > MAX_IO_TIMEOUT)
            throw ConfigException("IO Timeout can't be less than %d and more than %d",
                                  MIN_IO_TIMEOUT, MAX_IO_TIMEOUT);
        this->ioTimeout_ = ioTimeout;
    }

    long getInactivityTime() const {
        return inactivityTime_;
    }
    void setInactivityTime(long inactivityTime) throw(ConfigException) {
        if (inactivityTime < MIN_INACTIVITY_TIME || inactivityTime > MAX_INACTIVITY_TIME)
            throw ConfigException("Inactivity time can't be less than %ld and more than %ld msecs",
                                  MIN_INACTIVITY_TIME, MAX_INACTIVITY_TIME );
        this->inactivityTime_ = inactivityTime;
    }
    long getConnectTimeout() const {
        return connectTimeout_;
    }
    void setConnectTimeout(long connectTimeout) throw(ConfigException) {
        if (connectTimeout < MIN_CONNECT_TIMEOUT || connectTimeout > MAX_CONNECT_TIMEOUT)
            throw ConfigException("Connect timeout count can't be less than %ld and more than %ld",
                                  MIN_CONNECT_TIMEOUT, MAX_CONNECT_TIMEOUT);
        this->connectTimeout_ = connectTimeout;
    }
    long getProcessTimeout() const {
        return processTimeout_;
    }
    void setProcessTimeout(long processTimeout) throw(ConfigException) {
        if (processTimeout < MIN_PROCESS_TIMEOUT || processTimeout > MAX_PROCESS_TIMEOUT)
            throw ConfigException("Process timeout count can't be less than %ld and more than %ld",
                                  MIN_PROCESS_TIMEOUT, MAX_PROCESS_TIMEOUT);
        this->processTimeout_ = processTimeout;
    }

    // TODO: check limits !!!
    int getMaxReaderChannelsCount() const {
        return maxReaderChannelsCount_;
    }
    void setMaxReaderChannelsCount(int maxReaderChannelsCount) {
        this->maxReaderChannelsCount_ = maxReaderChannelsCount;
    }

    int getMaxWriterChannelsCount() const {
        return maxWriterChannelsCount_;
    }
    void setMaxWriterChannelsCount(int maxWriterChannelsCount) {
        this->maxWriterChannelsCount_ = maxWriterChannelsCount;
    }

    int getReadersCount() const {
        return readersCount_;
    }
    void setReadersCount(int readersCount) {
        this->readersCount_ = readersCount;
    }

    int getWritersCount() const {
        return writersCount_;
    }
    void setWritersCount(int writersCount) {
        this->writersCount_ = writersCount;
    }

    Config() :
    channelQueueSizeLimit_(DEF_CHANNEL_QUEUE_SIZE),
    packetSizeLimit_(MAX_PACKET_SIZE),
    readersCount_(0),
    writersCount_(0),
    maxReaderChannelsCount_(0),
    maxWriterChannelsCount_(0),
    port_(0),
    ioTimeout_(DEF_IO_TIMEOUT),
    inactivityTime_(DEFAULT_INACTIVITY_TIME),
    connectTimeout_(DEF_CONNECT_TIMEOUT),
    processTimeout_(DEF_PROCESS_TIMEOUT)
    {}

private:
    int channelQueueSizeLimit_;
    int packetSizeLimit_;

    int readersCount_;
    int writersCount_;
    int maxReaderChannelsCount_;
    int maxWriterChannelsCount_;

    std::string host_;
    short  port_;
    int    ioTimeout_; // timeout in msecs
    long   inactivityTime_; // ping time in msecs
    long   connectTimeout_;
    long   processTimeout_;
};

} // namespace core
} // namespace pvss
} // namespace scag2

namespace scag {
namespace pvss {
namespace core {

using scag2::pvss::core::Config;

}
}
}

#endif /* !_SCAG_PVSS_CORE_CONFIG_H */
