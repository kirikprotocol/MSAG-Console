#ifndef _INFORMER_COMMONSETTINGS_H
#define _INFORMER_COMMONSETTINGS_H

#include <string>
#include "informer/io/Typedefs.h"

namespace smsc {
namespace util {
namespace config {
class Config;
}
}
}

namespace eyeline {
namespace informer {

class UTF8;

class CommonSettings
{
    static CommonSettings* instance_;
public:
    inline static const CommonSettings* getCS() { return instance_; }

    CommonSettings();
    ~CommonSettings();

    // void init( const std::string& path, const std::string& statpath );
    void init( smsc::util::config::Config& cfg );

    // --- configurable parameters

    inline const std::string& getStorePath() const {
        return path_;
    }

    inline const std::string& getStatPath() const {
        return statpath_;
    }

    inline const char* getSvcType() const { return svcType_.c_str(); }

    /// how many seconds to wait between input journal rolling
    inline unsigned getInputJournalRollingPeriod() const {
        return inputJournalRollingPeriod_;
    }

    /// a number of messages (low mark) when input upload should start.
    inline unsigned getInputMinQueueSize() const {
        return inputMinQueueSize_;
    }

    /// how many messages to upload from input storage in one chunk.
    inline unsigned getInputTransferChunkSize() const {
        return inputTransferChunkSize_;
    }

    inline unsigned getInputTransferThreadCount() const {
        return inputTransferThreadCount_;
    }

    /// how many seconds to wait between input journal rolling
    inline unsigned getOpJournalRollingPeriod() const {
        return operationalJournalRollingPeriod_;
    }

    inline unsigned getProtocolId() const { return protocolId_; }

    /// how many seconds to keep waiting receipt after its validity period expired.
    inline timediff_type getReceiptExtraWaitTime() const {
        return timediff_type(receiptExtraWaitTime_);
    }

    inline unsigned getResendIOThreadCount() const {
        return resendIOThreadCount_;
    }

    inline unsigned getResponseWaitTime() const {
        return responseWaitTime_;
    }

    /// minimal retry interval allowed to 
    inline timediff_type getRetryMinTimeToLive() const {
        return timediff_type(retryMinTimeToLive_);
    }

    inline unsigned getSkippedIdsChunkSize() const {
        return skippedIdsChunkSize_;
    }

    /// get minimum black list message count in cache
    inline unsigned getSkippedIdsMinCacheSize() const {
        return skippedIdsMinCacheSize_;
    }

    inline unsigned getSlicedMessageSize() const {
        return slicedMessageSize_;
    }

    /// maximum number of unresponded messages per SMSC
    inline unsigned getUnrespondedMessagesMax() const {
        return unrespondedMessagesMax_; 
    }

    /// default validity period for each message (initial TTL)
    inline timediff_type getValidityPeriodDefault() const {
        return timediff_type(validityPeriodDefault_);
    }

    // --- non-configurable parameters

    /// return a length of period (second) to split resend messages into.
    /// must state: assert(3600%ret==0), assert(ret%60==0).
    inline msgtime_type getResendUploadPeriod() const {
        return 600;
    }

    /// upload next resend file if less than this time remains.
    inline msgtime_type getResendMinTimeToUpload() const {
        return 60;
    }

    /// incremental statistics bank index
    inline unsigned getStatBankIndex() const { return incStatBank_; }

    /// flip statistics bank, no locking here
    void flipStatBank() {
        incStatBank_ = 1 - incStatBank_;
    }

    UTF8& getUTF8() const { return *utf8_; }

private:
    std::string path_;
    std::string statpath_;
    UTF8*       utf8_;        // owned converter
    unsigned    incStatBank_;

    std::string svcType_;
    unsigned inputJournalRollingPeriod_;
    unsigned inputMinQueueSize_;
    unsigned inputTransferChunkSize_;
    unsigned inputTransferThreadCount_;
    unsigned operationalJournalRollingPeriod_;
    unsigned protocolId_;
    unsigned receiptExtraWaitTime_;
    unsigned resendIOThreadCount_;
    unsigned responseWaitTime_;
    unsigned retryMinTimeToLive_;
    unsigned skippedIdsChunkSize_;
    unsigned skippedIdsMinCacheSize_;
    unsigned slicedMessageSize_;
    unsigned unrespondedMessagesMax_;
    unsigned validityPeriodDefault_;
};

inline const CommonSettings* getCS() { return CommonSettings::getCS(); }

} // informer
} // smsc

#endif
