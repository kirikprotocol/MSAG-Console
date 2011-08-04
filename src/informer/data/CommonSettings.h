#ifndef _INFORMER_COMMONSETTINGS_H
#define _INFORMER_COMMONSETTINGS_H

#include <string>
#include <vector>
#include <map>
#include "informer/io/Typedefs.h"

namespace smsc {
namespace logger {
class Logger;
}
namespace util {
namespace config {
class Config;
}
}
}

namespace eyeline {
namespace informer {

class UTF8;
class TimezoneGroup;
class SnmpManager;
class DeadLockWatcher;

class CommonSettings
{
    static CommonSettings* instance_;
public:
    inline static const CommonSettings* getCS() { return instance_; }

    CommonSettings( unsigned licenseLimit );
    ~CommonSettings();

    void init( smsc::util::config::Config& cfg,
               SnmpManager*                snmp,       // not owned
               bool                        archive );

    inline bool isArchive() const {
        return archive_;
    }

    inline bool isEmergency() const {
        return emergency_;
    }

    inline bool isStopping() const {
        return stopping_;
    }

    void setStopping();
    // stopping_ = true;
    // }

    // --- configurable parameters

    inline const std::string& getStorePath() const {
        return path_;
    }

    inline const std::string& getArchivePath() const {
        return archivePath_;
    }

    inline const std::string& getStatPath() const {
        return statpath_;
    }

    inline const char* getSvcType() const { return svcType_.c_str(); }

    /// how many seconds to wait between input journal rolling
    inline unsigned getInputJournalRollingPeriod() const {
        return inputJournalRollingPeriod_;
    }

    // a number of messages (low mark) when input upload should start.
    // inline unsigned getInputMinQueueSize() const {
    // return inputMinQueueSize_;
    // }

    /// a number of seconds corresponding to input queue (low mark)
    /// when input upload should start.
    inline unsigned getInputMinQueueTime() const {
        return inputMinQueueTime_;
    }

    /// how many seconds should cover the messages uploaded from input storage
    /// in one chunk.
    inline unsigned getInputTransferChunkTime() const {
        return inputTransferChunkTime_;
    }

    inline unsigned getInputTransferThreadCount() const {
        return inputTransferThreadCount_;
    }

    inline unsigned getInputRequestAverageTime() const {
        return inputRequestAverageTime_;
    }

    /// how many seconds to wait between input journal rolling
    inline unsigned getOpJournalRollingPeriod() const {
        return operationalJournalRollingPeriod_;
    }

    inline unsigned getSmscJournalRollingPeriod() const {
        return smscJournalRollingPeriod_;
    }

    inline unsigned getInputJournalRollingSpeed() const {
        return inputJournalRollingSpeed_;
    }
    inline unsigned getOpJournalRollingSpeed() const {
        return operationalJournalRollingSpeed_;
    }
    inline unsigned getSmscJournalRollingSpeed() const throw() {
        return smscJournalRollingSpeed_;
    }

    inline unsigned getProtocolId() const throw() { return protocolId_; }

    /// how many seconds to keep waiting receipt after its validity period expired.
    inline timediff_type getReceiptExtraWaitTime() const throw() {
        return timediff_type(receiptExtraWaitTime_);
    }

    inline unsigned getResponseWaitTime() const throw() {
        return responseWaitTime_;
    }

    /// minimal retry interval allowed to 
    inline timediff_type getRetryMinTimeToLive() const throw() {
        return timediff_type(retryMinTimeToLive_);
    }

    inline unsigned getSkippedIdsChunkSize() const throw() {
        return skippedIdsChunkSize_;
    }

    /// get minimum black list message count in cache
    inline unsigned getSkippedIdsMinCacheSize() const throw() {
        return skippedIdsMinCacheSize_;
    }

    inline unsigned getSlicedMessageSize() const throw() {
        return slicedMessageSize_;
    }

    /// maximum number of unresponded messages per SMSC
    inline unsigned getUnrespondedMessagesMax() const throw() {
        return unrespondedMessagesMax_; 
    }

    /// default validity period for each message (initial TTL)
    inline timediff_type getValidityPeriodDefault() const throw() {
        return timediff_type(validityPeriodDefault_);
    }

    /// default validity period for each message (initial TTL)
    inline timediff_type getMessageTimeToLiveDefault() const throw() {
        return timediff_type(messageTimeToLiveDefault_);
    }

    inline unsigned getLicenseLimit() const throw() {
        return licenseLimit_;
    }

    inline unsigned getResendIOThreadCount() const throw() {
        return resendIOThreadCount_;
    }

    inline unsigned getResendQueueMaxSize() const throw() {
        return 100;
    }

    inline unsigned getDlvCacheSize() const throw() {
        return 0;
    }

    inline unsigned getRegionRandomizationJitter() const throw() {
        return regionRandomizationJitter_;
    }

    // inline unsigned getRegionSpeedLimitNBins() const throw() {
    // return regionSpeedLimitNBins_;
    // }
    // inline unsigned getRegionSpeedLimitInterval() const throw() {
    // return regionSpeedLimitInterval_;
    // }

    inline unsigned getRegionSpeedLimitSpeedup() const throw() {
        return regionSpeedLimitSpeedup_;
    }

    // inline unsigned getRegionSpeedLimitBaseSpeed() const throw() {
    // return regionSpeedLimitBaseSpeed_;
    // }

    inline bool isRecalcTTLRequested() const {
        return recalcTTLRequested_;
    }

    // --- non-configurable parameters

    inline msgtime_type getDeadLockWatchPeriod() const {
        return 60;
    }

    /// return a length of period (second) to split resend messages into.
    /// must state: assert(3600%ret==0), assert(ret%60==0).
    /// NOTE: never changes this value unless you don't have resends!
    inline msgtime_type getResendUploadPeriod() const {
        return 300;
    }

    /// upload next resend file if less than this time remains.
    inline msgtime_type getResendMinTimeToUpload() const {
        return 60;
    }

    inline unsigned getInputStorageFileSize() const {
        return 20000;
    }

    /// incremental statistics bank index
    inline unsigned getStatBankIndex() const { return incStatBank_; }

    /// flip statistics bank, no locking here
    void flipStatBankIndex() {
        incStatBank_ = 1 - incStatBank_;
    }

    UTF8& getUTF8() const { return *utf8_; }

    /// get timezone group
    const TimezoneGroup* lookupTimezoneGroup( const char* tzname ) const;

    inline SnmpManager* getSnmp() const { return snmp_; }

    inline DeadLockWatcher& getDLWatcher() const { return *dlwatcher_; }

private:
    void loadTimezones();
    typedef std::map< std::string, TimezoneGroup* > TzMap;

private:
    smsc::logger::Logger* log_;
    std::string     path_;
    std::string     archivePath_;
    std::string     statpath_;
    UTF8*           utf8_;        // owned converter
    SnmpManager*     snmp_;
    DeadLockWatcher* dlwatcher_;
    unsigned         incStatBank_;

    std::string svcType_;
    unsigned licenseLimit_;
    unsigned inputJournalRollingPeriod_;
    unsigned inputMinQueueTime_;
    unsigned inputTransferChunkTime_;
    unsigned inputTransferThreadCount_;
    unsigned inputRequestAverageTime_;
    unsigned operationalJournalRollingPeriod_;
    unsigned smscJournalRollingPeriod_;
    unsigned inputJournalRollingSpeed_;
    unsigned operationalJournalRollingSpeed_;
    unsigned smscJournalRollingSpeed_;
    unsigned protocolId_;
    unsigned receiptExtraWaitTime_;
    unsigned resendIOThreadCount_;
    unsigned regionRandomizationJitter_;
    unsigned responseWaitTime_;
    unsigned retryMinTimeToLive_;
    unsigned skippedIdsChunkSize_;
    unsigned skippedIdsMinCacheSize_;
    unsigned slicedMessageSize_;
    unsigned unrespondedMessagesMax_;
    unsigned validityPeriodDefault_;
    unsigned messageTimeToLiveDefault_;
    // unsigned regionSpeedLimitNBins_;
    // unsigned regionSpeedLimitInterval_;
    unsigned regionSpeedLimitSpeedup_;
    // unsigned regionSpeedLimitBaseSpeed_;
    bool     recalcTTLRequested_;

    volatile bool stopping_;
    bool     archive_;
    bool     emergency_;

    std::vector< TimezoneGroup* > tzgroups_;
    TzMap                         tzmap_;
};

inline const CommonSettings* getCS() { return CommonSettings::getCS(); }

} // informer
} // smsc

#endif
