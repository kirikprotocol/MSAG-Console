#ifndef _INFORMER_DELIVERYINFO_H
#define _INFORMER_DELIVERYINFO_H

#include "informer/io/Typedefs.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

class CommonSettings;

class DeliveryInfo
{
public:
    DeliveryInfo( const CommonSettings& cs,
                  dlvid_type dlvId ) :
    cs_(cs), dlvId_(dlvId), state_(DLVSTATE_PAUSED) {
        if (!log_) log_ = smsc::logger::Logger::getInstance("dlvinfo");
    }

    dlvid_type getDlvId() const { return dlvId_; }
    unsigned getPriority() const { return 1; }

    bool isActive() const {
        return (state_ == DLVSTATE_ACTIVE);
    }

    /// minimal number of input messages per region when request for new
    /// input messages should be issued.
    unsigned getMinInputQueueSize() const { return 5; }

    /// maximum number of messages in resend queue that prevents the
    /// request for new input messages.
    unsigned getMaxResendQueueSize() const { return 20; }

    /// number of input messages to be requested.
    unsigned getUploadCount() const { return 10; }

    /// minimal time between retries, seconds
    unsigned getMinRetryTime() const { return 60; }

    /// message validity time, seconds
    unsigned getMessageValidityTime() const { return 3600; }

    /// read delivery info
    void read();

private:
    static smsc::logger::Logger* log_;

private:
    const CommonSettings& cs_;
    dlvid_type      dlvId_;
    DlvState        state_;
};

} // informer
} // smsc

#endif
