#ifndef _INFORMER_DELIVERYINFO_H
#define _INFORMER_DELIVERYINFO_H

#include "informer/io/Typedefs.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

class CommonSettings;

typedef enum {
    DLVMODE_SMS = 0,
    DLVMODE_USSDPUSH = 1,
    DLVMODE_USSDPUSHVLR = 2
} DeliveryMode;


class DeliveryInfo
{
public:
    DeliveryInfo( const CommonSettings& cs,
                  dlvid_type dlvId ) :
    cs_(cs), dlvId_(dlvId),
    from_(0x4010000000000000ULL + 10000), // FIXME: .0.1.10000
    isReplaceIfPresent_(true), isFlash_(false), useDataSm_(false),
    transactionMode_(0),
    deliveryMode_(DLVMODE_SMS),
    state_(DLVSTATE_PAUSED) {
        if (!log_) log_ = smsc::logger::Logger::getInstance("dlvinfo");
    }

    dlvid_type getDlvId() const { return dlvId_; }
    unsigned getPriority() const { return 1; }

    bool isActive() const {
        return (state_ == DLVSTATE_ACTIVE);
    }

    personid_type getFrom() const { return from_; } 

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

    bool isReplaceIfPresent() const { return isReplaceIfPresent_; }

    const std::string& getSvcType() const { return svcType_; }

    int getTransactionMode() const { return transactionMode_; }

    bool isFlash() const { return isFlash_; }

    bool useDataSm() const { return useDataSm_; }

    DeliveryMode getDeliveryMode() const { return deliveryMode_; }

    /// read delivery info
    void read();

private:
    static smsc::logger::Logger* log_;

private:
    const CommonSettings& cs_;
    dlvid_type      dlvId_;
    personid_type   from_;
    bool            isReplaceIfPresent_, isFlash_, useDataSm_;
    std::string     svcType_;
    int             transactionMode_;
    DeliveryMode    deliveryMode_;
    DlvState        state_;
};

} // informer
} // smsc

#endif
