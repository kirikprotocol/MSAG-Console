#include <cassert>
#include <cstring>
#include <vector>
#include "informer/io/InfosmeException.h"
#include "DeliveryInfo.h"
#include "CommonSettings.h"

namespace {

using namespace eyeline::informer;

int parseWeekDays( const std::vector< std::string >& wd )
{
    static const char* fulldays[7] = {
        "Monday", "Tuesday", "Wednesday", "Thursday",
        "Friday", "Saturday", "Sunday"
    };
    static const char* shortdays[7] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };

    int res = 0;
    for ( std::vector< std::string >::const_iterator i = wd.begin();
          i != wd.end(); ++i ) {
        bool ok = false;
        int x = 1;
        for ( int j = 0; j < 7; ++j ) {
            if ( 0 == strcmp(fulldays[j],i->c_str()) ||
                 0 == strcmp(shortdays[j],i->c_str()) ) {
                ok = true;
                res |= x;
                break;
            }
            x *= 2;
        }
        if (!ok) {
            throw InfosmeException(EXC_IOERROR,"wrong weekday '%s'",i->c_str());
        }
    }
    return res;
}

}

namespace eyeline {
namespace informer {

smsc::logger::Logger* DeliveryInfo::log_ = 0;

void DeliveryInfo::update( const DeliveryInfoData& data )
{
    throw InfosmeException(EXC_NOTIMPL, "DeliveryInfo::update(): not impl");
}


DeliveryInfo::DeliveryInfo( const CommonSettings&   cs,
                            dlvid_type              dlvId,
                            const DeliveryInfoData& data ) :
cs_(cs),
dlvId_(dlvId),
startDate_(0),
endDate_(0),
activePeriodStart_(-1),
activePeriodEnd_(-1),
validityPeriod_(-1),
activeWeekDays_(-1),
sourceAddress_(0)
{
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("dlvinfo");
    }
    updateData( data, 0 );
}


void DeliveryInfo::updateData( const DeliveryInfoData& data,
                               const DeliveryInfoData* old )
{
    msgtime_type startDate = startDate_;
    msgtime_type endDate = endDate_;
    timediff_type activePeriodStart = activePeriodStart_;
    timediff_type activePeriodEnd = activePeriodEnd_;
    timediff_type validityPeriod = validityPeriod_;
    int activeWeekDays = activeWeekDays_;
    personid_type sourceAddress = sourceAddress_;

    if (!old && !data.startDate.empty()) { // calculate only at start
        startDate = parseDateTime(data.startDate.c_str());
    }
    if ((!old || old->endDate != data.endDate) && !data.endDate.empty() ) {
        endDate = parseDateTime(data.endDate.c_str());
    }
    if ((!old || old->activePeriodStart != data.activePeriodStart) && !data.activePeriodStart.empty() ) {
        activePeriodStart = parseTime(data.activePeriodStart.c_str());
    }
    if ((!old || old->activePeriodEnd != data.activePeriodEnd) && !data.activePeriodEnd.empty()) {
        activePeriodEnd = parseTime(data.activePeriodEnd.c_str());
    }
    if ((!old || old->activeWeekDays != data.activeWeekDays) && !data.activeWeekDays.empty() ) {
        activeWeekDays = parseWeekDays(data.activeWeekDays);
    }
    if ((!old || old->validityPeriod != data.validityPeriod) && !data.validityPeriod.empty() ) {
        validityPeriod = parseTime(data.validityPeriod.c_str());
    }
    // FIXME: update retry policy: data.retryPolicy
    if ((!old || old->sourceAddress != data.sourceAddress) && !data.sourceAddress.empty()) {
        sourceAddress = parseAddress(data.sourceAddress.c_str());
    }

    if ( !isGoodAsciiName(data.userData.c_str()) ) {
        throw InfosmeException(EXC_BADNAME,"invalid chars in userData '%s'",data.userData.c_str());
    }

    /// post-parsing check & fill
    if ( ( activePeriodStart < 0 && activePeriodEnd >= 0 ) ||
         ( activePeriodStart >= 0 && activePeriodEnd < 0 ) ) {
        throw InfosmeException(EXC_CONFIG,"invalid active period start/end");
    }

    if ( sourceAddress == 0 ) {
        throw InfosmeException(EXC_CONFIG,"source address in not specified");
    }

    // filling
    if (startDate != 0) { startDate_ = startDate; }
    if (endDate != 0) { endDate_ = endDate; }
    if (activePeriodStart != -1) { activePeriodStart_ = activePeriodStart; }
    if (activePeriodEnd != -1) { activePeriodEnd_ = activePeriodEnd; }
    if (validityPeriod != -1) { validityPeriod_ = validityPeriod; }
    if (activeWeekDays != -1) { activeWeekDays_ = activeWeekDays; }
    if (sourceAddress != 0) { sourceAddress_ = sourceAddress; }
    data_ = data;
}

}
}
