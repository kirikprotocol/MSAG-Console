#ifndef _INFORMER_DELIVERYINFO_H
#define _INFORMER_DELIVERYINFO_H

#include <vector>
#include "informer/io/Typedefs.h"
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"
#include "RetryString.h"

namespace eyeline {
namespace informer {

class CommonSettings;

enum DlvMode{
    DLVMODE_SMS = 0,
    DLVMODE_USSDPUSH = 1,
    DLVMODE_USSDPUSHVLR = 2
};


/// the structure holding data from dcp protocol.
struct DeliveryInfoData
{
  std::string name;
  int32_t priority;
  bool transactionMode;
  // DD.MM.YY hh:mm:ss (gmt)
  std::string startDate;
  std::string endDate;
  std::string activePeriodStart;
  std::string activePeriodEnd;
  std::vector<std::string> activeWeekDays;
  std::string validityPeriod;
  bool flash;
  bool useDataSm;
  DlvMode deliveryMode;
  std::string owner;
  bool retryOnFail;
  std::string retryPolicy;
  bool replaceMessage;
  std::string svcType;
  std::string userData;
  std::string sourceAddress;
  bool finalDlvRecords;
  bool finalMsgRecords;
};


class DeliveryInfo
{
public:
    // constructor from file system
    DeliveryInfo( const CommonSettings&   cs,
                  dlvid_type              dlvId,
                  const DeliveryInfoData& data );

    const CommonSettings& getCS() const { return cs_; }

    dlvid_type getDlvId() const { return dlvId_; }

    void update( const DeliveryInfoData& data );

    const DeliveryInfoData& getDeliveryData() const
    {
      return data_;
    }

    // ============ delivery settings ==========================

    const char* getName() const { return data_.name.c_str(); }

    unsigned getPriority() const { return data_.priority; }

    bool isTransactional() const { return data_.transactionMode; }

    bool wantFinalMsgRecords() const { return data_.finalMsgRecords; }
    bool wantFinalDlvRecords() const { return data_.finalDlvRecords; }

    /// return start date or 0
    msgtime_type getStartDate() const { return startDate_; }

    /// return end date or 0
    msgtime_type getEndDate() const { return endDate_; }

    /// return active period start or -1
    timediff_type getActivePeriodStart() const { return activePeriodStart_; }

    /// return active period end or -1
    timediff_type getActivePeriodEnd() const { return activePeriodEnd_; }

    /// return active week days or -1
    int getActiveWeekDays() const { return activeWeekDays_; }

    /// the method calculates the next activation time for the delivery.
    /// @param tm_wday - the value of the field of the struct tm.
    /// @param now - current gm time.
    /// @return the activation time, values:
    ///  >0 how many seconds to wait until activation;
    ///  =0 if already in the active period;
    timediff_type nextActiveTime( int tm_wday, msgtime_type now ) const;

    /// the method calculates the next stop time for the delivery.
    /// @param tm_wday - the value of the field of the struct tm.
    /// @param now - current gm time.
    /// @return the stop time, values:
    ///   >0 how many seconds to wait until stop;
    ///   =0 if already in the stop period;
    ///   <0 if there is no active period/active weekdays limits.
    timediff_type nextStopTime( int tm_wday, msgtime_type now ) const;

    /// get validity period or -1
    timediff_type getValidityPeriod() const { return validityPeriod_; }

    bool isFlash() const { return data_.flash; }

    bool useDataSm() const { return data_.useDataSm; }

    DlvMode getDeliveryMode() const { return data_.deliveryMode; }

    // if retry on fail is requested.
    bool wantRetryOnFail() const { return data_.retryOnFail; }
    timediff_type getRetryInterval( uint16_t retryCount ) const {
        return retryPolicy_.getRetryInterval(retryCount);
    }

    bool isReplaceIfPresent() const { return data_.replaceMessage; }

    const char* getSvcType() const { return data_.svcType.c_str(); }

    const char* getUserData() const { return data_.userData.c_str(); }

    personid_type getSourceAddress() const { return sourceAddress_; }

    // ============ end of delivery settings ==========================

protected:
    /// update cached fields from data
    void updateData( const DeliveryInfoData& data,
                     const DeliveryInfoData* old );

private:
    static smsc::logger::Logger* log_;

private:
    const CommonSettings& cs_;
    dlvid_type            dlvId_;
    DeliveryInfoData      data_;

    // cached things updated from data_
    msgtime_type          startDate_;
    msgtime_type          endDate_;
    timediff_type         activePeriodStart_;
    timediff_type         activePeriodEnd_;
    timediff_type         validityPeriod_;
    int                   activeWeekDays_;
    personid_type         sourceAddress_;
    RetryString           retryPolicy_;
};

} // informer
} // smsc

#endif
