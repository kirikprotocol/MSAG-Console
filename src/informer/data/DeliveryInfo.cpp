#include <cassert>
#include <cstring>
#include <vector>
#include "informer/io/InfosmeException.h"
#include "DeliveryInfo.h"
#include "CommonSettings.h"

namespace eyeline {
namespace informer {

namespace {

/// bit values for week days
static const int weekBits[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40 };

static const timediff_type daynight = 24*3600;

int parseWeekDays( const std::vector< std::string >& wd )
{
    static const char* days[] = {
        "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday",
        "monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday",
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun",
        "mon", "tue", "wed", "thu", "fri", "sat", "sun",
        0
    };

    int res = 0;
    for ( std::vector< std::string >::const_iterator i = wd.begin();
          i != wd.end(); ++i ) {
        bool ok = false;
        for ( const char** p = days; *p != 0; ++p ) {
            if ( 0 == strcmp(*p,i->c_str()) ) {
                const unsigned idx = unsigned(p - days) % 7;
                ok = true;
                res |= weekBits[idx];
                break;
            }
        }
        if (!ok) {
            throw InfosmeException(EXC_IOERROR,"wrong weekday '%s'",i->c_str());
        }
    }
    return res;
}

} // namespace

smsc::logger::Logger* DeliveryInfo::log_ = 0;

DeliveryInfo::DeliveryInfo( dlvid_type              dlvId,
                            const DeliveryInfoData& data ) :
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


void DeliveryInfo::update( const DeliveryInfoData& data )
{
    updateData( data, &data_);
}


bool DeliveryInfo::checkActiveTime( int weekTime ) const
{
    if (activeWeekDays_ != -1) {
        const unsigned weekDay = unsigned(weekTime / daynight);
        if ( weekDay>= 7 ) {
            throw InfosmeException(EXC_LOGICERROR,"D=%u wrong weekTime=%d -> day=%u",dlvId_,weekTime,weekDay);
        }
        if ( (activeWeekDays_ & weekBits[weekDay]) == 0 ) {
            smsc_log_debug(log_,"D=%u checkActive(%u): weekday=%u disabled",dlvId_,weekTime,weekDay);
            return false;
        }
    }
    if ( activePeriodStart_ >= 0 ) {
        const timediff_type dayTime = weekTime % daynight;
        if ( activePeriodStart_ < activePeriodEnd_ ) {
            if (dayTime < activePeriodStart_ || dayTime >= activePeriodEnd_ ) {
                smsc_log_debug(log_,"D=%u checkActive(%u): daytime=%u as=%u ae=%u",
                               dlvId_, weekTime, dayTime, activePeriodStart_, activePeriodEnd_);
                return false;
            }
        } else {
            if (dayTime < activePeriodStart_ && dayTime >= activePeriodEnd_ ) {
                smsc_log_debug(log_,"D=%u checkActive(%u): daytime=%u as=%u ae=%u",
                               dlvId_, weekTime, dayTime, activePeriodStart_, activePeriodEnd_);
                return false;
            }
        }
    }
    return true;
}


/*
timediff_type DeliveryInfo::nextActiveTime( int tm_wday, msgtime_type now ) const
{
    if ((activeWeekDays_ & 0x6f) == 0) {
        throw InfosmeException(EXC_LOGICERROR,"wrong active week days: %d",activeWeekDays_);
    }
    // current day time
    timediff_type ms = timediff_type(now % daynight);
    timediff_type res = 0;
    const char* what = "";
    timediff_type fixedAE = activePeriodEnd_ - 60; // one minute less
    if (fixedAE<0) { fixedAE += daynight; }
    for (;;) {

        smsc_log_debug(log_,"D=%u acttime wday=%u ms=%u as=%d ae=%d res=%u",
                       dlvId_, tm_wday, ms, activePeriodStart_, fixedAE, res );

        if ((weekBits[tm_wday] & activeWeekDays_)!=0) {
            // day is allowed
            if (activePeriodStart_<0) {
                // not limited by period
                what = "all day allowed";
                break;
            }

            if (ms<activePeriodStart_) {
                if (fixedAE < activePeriodStart_ && ms < fixedAE ) {
                    what = "before invend";
                    break;
                } else {
                    what = "before start";
                    res += activePeriodStart_ - ms;
                    break;
                }
            } else if (fixedAE < activePeriodStart_) {
                what = "after invstart";
                break;
            }
        }

        // move to the next day
        res += (daynight - ms);
        ms = 0;
        if ( ++tm_wday >= 7 ) {tm_wday = 0;}

    }
    smsc_log_debug(log_,"D=%u acttime final (%s) res=%u",dlvId_,what,res);
    return res;
}


timediff_type DeliveryInfo::nextStopTime( int tm_wday, msgtime_type now ) const
{
    if ( (activeWeekDays_ & 0x6f) == 0x6f &&
         activePeriodStart_<0 ) {
        // not limited
        return -1;
    }
    timediff_type ms = timediff_type(now % daynight);
    timediff_type fixedAE = activePeriodEnd_ - 60;
    if (fixedAE<0) { fixedAE += daynight; }
    timediff_type res = 0;
    const char* what = "";
    for (;;) {

        smsc_log_debug(log_,"D=%u stoptime wday=%u ms=%u as=%d ae=%d res=%u",
                       dlvId_, tm_wday, ms, activePeriodStart_, fixedAE, res );

        if ((weekBits[tm_wday] & activeWeekDays_)==0) {
            // day is not allowed
            what = "all day forbidden";
            break;
        }
        // day is allowed
        if (activePeriodStart_>=0) {
            if (ms < fixedAE) {
                if (activePeriodStart_<fixedAE && ms < activePeriodStart_) {
                    what = "before start";
                    break;
                } else {
                    res += fixedAE - ms;
                    what = "before end";
                    break;
                }
            } else if (ms < activePeriodStart_) {
                what = "before invstart";
                break;
            }
        }

        // move to the next day
        res += (daynight - ms);
        ms = 0;
        if (++tm_wday>=7) {tm_wday = 0;}
    }
    smsc_log_debug(log_,"D=%u stoptime final (%s) res=%u",dlvId_,what,res);
    return res;
}
 */


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
    RetryString retryPolicy;
    bool newRetryPolicy = false;

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
    if (!old || old->retryPolicy != data.retryPolicy) {
        retryPolicy.init(data.retryPolicy.c_str());
        newRetryPolicy = true;
    }
    if ((!old || old->sourceAddress != data.sourceAddress) && !data.sourceAddress.empty()) {
        sourceAddress = parseAddress(data.sourceAddress.c_str());
    }

    if ( !isGoodAsciiName(data.userData.c_str()) ) {
        throw InfosmeException(EXC_BADNAME,"invalid chars in userData '%s'",data.userData.c_str());
    }

    /// post-parsing check & fill
    if (data.priority < 1 || data.priority > 100 ) {
        throw InfosmeException(EXC_CONFIG,"invalid priority %d",data.priority);
    }

    if ( ( activePeriodStart < 0 && activePeriodEnd >= 0 ) ||
         ( activePeriodStart >= 0 && activePeriodEnd < 0 ) ) {
        throw InfosmeException(EXC_CONFIG,"invalid active period start/end");
    } else if ( activePeriodStart >= 0 ) {
        activePeriodStart %= daynight;
        activePeriodEnd %= daynight;
        timediff_type diff = activePeriodEnd - activePeriodStart;
        if (diff < 0) diff += daynight;
        if (diff <= 60) {
            throw InfosmeException(EXC_CONFIG,"too small active period AS=%d AE=%d",
                                   activePeriodStart, activePeriodEnd);
        }
        if (diff >= daynight+60) {
            throw InfosmeException(EXC_CONFIG,"too big active period AS=%d AE=%d",
                                   activePeriodStart, activePeriodEnd);
        }
    }

    if ( sourceAddress == 0 ) {
        throw InfosmeException(EXC_CONFIG,"source address in not specified");
    }

    // filling
    if (startDate != 0) {
        startDate_ = startDate;
    }
    if (endDate != 0) { endDate_ = endDate; }
    if (activePeriodStart != -1) { activePeriodStart_ = activePeriodStart; }
    if (activePeriodEnd != -1) { activePeriodEnd_ = activePeriodEnd; }
    if (validityPeriod != -1) { validityPeriod_ = validityPeriod; }
    if (activeWeekDays != -1) { activeWeekDays_ = activeWeekDays; }
    if (sourceAddress != 0) { sourceAddress_ = sourceAddress; }
    if (newRetryPolicy) { retryPolicy_ = retryPolicy; }
    data_ = data;
}

}
}
