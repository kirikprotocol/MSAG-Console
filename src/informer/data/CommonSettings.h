#ifndef _INFORMER_COMMONSETTINGS_H
#define _INFORMER_COMMONSETTINGS_H

#include <string>
#include "informer/io/Typedefs.h"

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

    void init( const std::string& path, const std::string& statpath );

    inline const std::string& getStorePath() const {
        return path_;
    }
    inline const std::string& getStatPath() const {
        return statpath_;
    }

    // msgtime_type getActivityLogPeriod() const { return 60; }

    // maximum number of unresponded messages per SMSC
    inline unsigned getUnrespondedMessagesMax() const { return unrespondedMessagesMax_; }

    inline unsigned getResponseWaitTime() const { return responseWaitTime_; }

    // inline unsigned getReceiptWaitTime() const { return receiptWaitTime_; }

    const char* getSvcType() const { return svcType_.c_str(); }

    unsigned getProtocolId() const { return protocolId_; }

    unsigned getSlicedMessageSize() const { return 160; /* FIXME */ }

    /// a number of messages (low mark) when input upload should start.
    unsigned getInputMinQueueSize() const {
        return 10;
    }

    /// how many messages to upload from input storage in one chunk.
    unsigned getInputTransferChunkSize() const {
        return 100;
    }

    /// how many seconds to keep waiting receipt after its validity period expired.
    timediff_type getReceiptExtraWaitTime() const {
        return 60;
    }

    /// return a length of period (second) to split resend messages into.
    /// must state: assert(3600%ret==0), assert(ret%60==0).
    /// NOTE: not configurable.
    msgtime_type getResendUploadPeriod() const {
        return 600;
    }

    /// upload next resend file if less than this time remains.
    /// NOTE: not configurable.
    msgtime_type getResendMinTimeToUpload() const {
        return 60;
    }

    // msgtime_type getStatDumpPeriod() const {
    // return 60;
    // }

    /// minimal retry interval allowed to 
    timediff_type getRetryMinTimeToLive() const {
        return 30;
    }

    /// get minimum black list message count in cache
    unsigned getSkippedIdsMinCacheSize() const {
        return 10;
    }
    unsigned getSkippedIdsChunkSize() const {
        return 256;
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
    unsigned    unrespondedMessagesMax_;
    unsigned    responseWaitTime_;
    unsigned    receiptWaitTime_;
    std::string svcType_;
    unsigned    protocolId_;
    unsigned    incStatBank_;
    UTF8*       utf8_;      // owned converter
};

inline const CommonSettings* getCS() { return CommonSettings::getCS(); }

} // informer
} // smsc

#endif
