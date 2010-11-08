#ifndef _INFORMER_DELIVERYINFO_H
#define _INFORMER_DELIVERYINFO_H

#include <vector>
#include "informer/io/Typedefs.h"
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace informer {

class CommonSettings;
// class UserInfo;
// class InfosmeCore;

enum DeliveryMode{
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
  // DD.MM.YY hh:mm:ss
  std::string startDate;
  std::string endDate;
  std::string activePeriodStart;
  std::string activePeriodEnd;
  std::vector<std::string> activeWeekDays;
  std::string validityPeriod;
  bool flash;
  bool useDataSm;
  DeliveryMode deliveryMode;
  std::string owner;
  bool retryOnFail;
  std::string retryPolicy;
  bool replaceMessage;
  std::string svcType;
  std::string userData;
  std::string sourceAddress;
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

    /// return start date or -1
    msgtime_type getStartDate() const { return startDate_; }

    /// return end date or -1
    msgtime_type getEndDate() const { return endDate_; }

    /// return active period start or -1
    timediff_type getActivePeriodStart() const { return activePeriodStart_; }

    /// return active period end or -1
    timediff_type getActivePeriodEnd() const { return activePeriodEnd_; }

    /// return active week days or -1
    int getActiveWeekDays() const { return activeWeekDays_; }

    /// get validity date or -1
    //msgtime_type getValidityDate() const { return validityDate_; }

    /// get validity period or -1
    msgtime_type getValidityPeriod() const { return validityPeriod_; }

    bool isFlash() const { return data_.flash; }

    bool useDataSm() const { return data_.useDataSm; }

    DeliveryMode getDeliveryMode() const { return data_.deliveryMode; }

    // FIXME: combine retry on fail together with retry policy string
    bool wantRetryOnFail() const { return data_.retryOnFail; }

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
    //msgtime_type          validityDate_;
    timediff_type         validityPeriod_;
    int                   activeWeekDays_;
    personid_type         sourceAddress_;
};

} // informer
} // smsc

#endif
