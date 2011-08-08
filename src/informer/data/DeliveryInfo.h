#ifndef _INFORMER_DELIVERYINFO_H
#define _INFORMER_DELIVERYINFO_H

#include <vector>
#include <map>
#include "informer/io/Typedefs.h"
#include "informer/io/EmbedRefPtr.h"
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/IntrList.hpp"
#include "core/buffers/FixedLengthString.hpp"
#include "RetryString.h"
#include "MessageGlossary.h"
#include "DeliveryStats.h"
#include "sms/sms.h"

namespace eyeline {
namespace informer {

class UserInfo;
class Region;

/// the structure holding data from dcp protocol.
struct DeliveryInfoData
{
  std::string name;
  int32_t priority;
  bool transactionMode;
  // DD.MM.YY hh:mm:ss (gmt)
  std::string startDate;
  std::string endDate;
  std::string creationDate;
  std::string activePeriodStart; // local time! HH:MM:SS or empty
  std::string activePeriodEnd;   // local time! HH:MM:SS or empty
  std::vector<std::string> activeWeekDays; // local time!
  std::string validityPeriod;
  std::string messageTimeToLive;
  std::string archivationPeriod; // HH:MM:SS or empty
  bool flash;
  bool useDataSm;
  DlvMode deliveryMode;
  std::string owner;
  bool retryOnFail;
  std::string retryPolicy;
  bool replaceMessage;
  std::string svcType;
  std::string dlvUserData;
  std::string sourceAddress;
  bool finalDlvRecords;
  bool finalMsgRecords;
  bool boundToLocalTime;
  std::string finalizationDelay; // HH:MM:SS or empty
};


class DeliveryInfo
{
public:
    struct IncStat {
        smsc::core::buffers::FixedLengthString< SMSC_ID_LENGTH > smscId;
        DeliveryAggregationStats stats;
        regionid_type regionId;
        IncStat*      next;
        IncStat() : next(0) {}
        ~IncStat() {
            clear();
        }
        inline void clear() {
            if (next) { delete next; next = 0; }
        }
        inline bool operator == ( regionid_type r ) const {
            return regionId == r;
        }
    };

    // constructor from file system
    DeliveryInfo( dlvid_type              dlvId,
                  const DeliveryInfoData& data,
                  UserInfo&               userInfo );

    dlvid_type getDlvId() const { return dlvId_; }

    void update( const DeliveryInfoData& data );

    inline const UserInfo& getUserInfo() const { return *userInfo_; }
    UserInfo& getUserInfo() { return *userInfo_; }

    inline void getDeliveryData( DeliveryInfoData& data ) const {
        smsc::core::synchronization::MutexGuard mg(lock_ MTXWHEREPOST);
        data = data_;
    }

    // ============ delivery settings ==========================

    /// get the name of the delivery
    inline void getName( char* name ) const {
        smsc::core::synchronization::MutexGuard mg(lock_ MTXWHEREPOST);
        strcpy(name,data_.name.c_str());
    }

    inline unsigned getPriority() const { return data_.priority; }

    inline bool isTransactional() const { return data_.transactionMode; }

    inline bool isBoundToLocalTime() const { return data_.boundToLocalTime; }

    inline bool wantFinalMsgRecords() const { return data_.finalMsgRecords; }
    inline bool wantFinalDlvRecords() const { return data_.finalDlvRecords; }

    /// return the start date
    inline msgtime_type getStartDate() const { return startDate_; }

    /// return end date or 0
    inline msgtime_type getEndDate() const { return endDate_; }

    /// return creation date or 0
    inline msgtime_type getCreationDate() const { return creationDate_; }

    /// return active period start in seconds since midnight (localtime) or -1
    inline timediff_type getActivePeriodStart() const { return activePeriodStart_; }

    /// return active period end in seconds since midnight (localtime) or -1
    inline timediff_type getActivePeriodEnd() const { return activePeriodEnd_; }

    /// return the number of seconds allowed to wait for new messages
    inline timediff_type getFinalizationDelay() const {
        return finalizationDelay_;
    }

    /// check activity time.
    /// @return number of seconds to wait until active period.
    ///  >0 -- how many seconds to wait until activeStart;
    ///  <=0 -- how many seconds left until activeEnd.
    int checkActiveTime( int weekTime ) const;

    /// recalculate TTL taking into account periods of task inactivity.
    /// @return actual value of TTL (>0 message is ok, <=0 message is expired).
    /// @param weekTime - week time of message last access (in local time);
    /// @param ttl - message ttl as it was at last access;
    /// @param uptonow - a number of seconds until now.
    timediff_type recalcTTL( int weekTime, timediff_type ttl, timediff_type uptonow ) const;

    /// evaluate the number of chunks in sms, fills sms fields (optional)
    unsigned evaluateNchunks( const char*     text,
                              size_t          textlen,
                              smsc::sms::SMS* sms = 0 ) const;

    /// get validity period or -1
    inline timediff_type getValidityPeriod() const { return validityPeriod_; }

    inline timediff_type getMessageTimeToLive() const { return messageTimeToLive_; }

    /// return the archivation time or <=0.
    /// the actual date is calculated from startDate.
    inline timediff_type getArchivationTime() const {
        return archivationTime_;
    }

    inline bool isFlash() const { return data_.flash; }

    inline bool useDataSm() const { return data_.useDataSm; }

    inline DlvMode getDeliveryMode() const { return data_.deliveryMode; }

    // if retry on fail is requested.
    inline bool wantRetryOnFail() const { return data_.retryOnFail; }
    timediff_type getRetryInterval( uint16_t retryCount ) const {
        smsc::core::synchronization::MutexGuard mg(lock_ MTXWHEREPOST);
        return retryPolicy_.getRetryInterval(retryCount);
    }

    inline bool isReplaceIfPresent() const { return data_.replaceMessage; }

    void getSvcType( char* svcType ) const {
        smsc::core::synchronization::MutexGuard mg(lock_ MTXWHEREPOST);
        strcpy(svcType,data_.svcType.c_str());
    }

    // const char* getUserData() const { return data_.userData.c_str(); }
    void getUserData( std::string& userData ) const {
        smsc::core::synchronization::MutexGuard mg(lock_ MTXWHEREPOST);
        userData = data_.dlvUserData;
    }

    void getSourceAddress( smsc::sms::Address& oa ) const {
        smsc::core::synchronization::MutexGuard mg(lock_ MTXWHEREPOST);
        oa = sourceAddress_;
    }

    // ============ end of delivery settings ==========================

    // --- stats
    void getMsgStats( DeliveryStats& ds ) const
    {
        smsc::core::synchronization::MutexGuard mg(statLock_);
        ds = stats_;
    }

    /// invoked at the initialization
    void initMsgStats( regionid_type regId,
                       uint8_t state,
                       int value );

    /// increment stats, optionally decrementing fromState
    void incMsgStats( const Region& region,
                      msgtime_type  currentTime,
                      uint8_t state,
                      int     value = 1,
                      uint8_t fromState = 0,
                      int     smsValue = 0 );

    inline msgtime_type getLastStatTime() const {
        return lastStatTime_;
    }

    /// the method pops released incremental stats.
    /// @return true if stats is filled.
    bool popMsgStats( IncStat& stats );

    /// glossary
    MessageGlossary& getGlossary() { return glossary_; }

    // FIXME: may be removed after all dlv converted
    // @return >0 if activity log format was fixed
    msgtime_type fixActLogFormat( msgtime_type currentTime );

    // FIXME: may be removed after all dlv fix their config.xml
    // @return >0 if creation date is reconstructed from activity log.
    static ulonglong fixCreationDate( dlvid_type dlvId );

protected:
    /// update cached fields from data
    void updateData( const DeliveryInfoData& data,
                     const DeliveryInfoData* old );

    /// read the statistics
    void readStats();

private:
    // static smsc::logger::Logger* log_;

private:
    smsc::logger::Logger*    log_;
    dlvid_type               dlvId_;
    EmbedRefPtr< UserInfo >  userInfo_;

    mutable smsc::core::synchronization::Mutex lock_;
    DeliveryInfoData                           data_;

    // cached things updated from data_
    msgtime_type          startDate_;
    msgtime_type          endDate_;
    msgtime_type          creationDate_;
    timediff_type         activePeriodStart_;
    timediff_type         activePeriodEnd_;
    timediff_type         validityPeriod_;
    timediff_type         messageTimeToLive_;
    timediff_type         archivationTime_;
    timediff_type         finalizationDelay_; // may be 0
    int                   activeWeekDays_;
    smsc::sms::Address    sourceAddress_;
    RetryString           retryPolicy_;

    MessageGlossary       glossary_;

    mutable smsc::core::synchronization::Mutex statLock_;
    DeliveryStats                      stats_;
    msgtime_type                       lastStatTime_; // not persistent

    struct StatNode : public smsc::core::buffers::IntrListNodeBase<StatNode, IncStat> {};
    typedef smsc::core::buffers::IntrList< StatNode > StatList;
    StatList statlist_[2];

    // FIXME: may be removed after all dlv converted
    bool                  isOldActLog_;
};

} // informer
} // smsc

#endif
