#include <cassert>
#include <cstring>
#include <vector>
#include "informer/io/InfosmeException.h"
#include "DeliveryInfo.h"
#include "CommonSettings.h"

namespace {

using namespace eyeline::informer;

/// bit value corresponding to the weekday from tm_wday.
static const int weekBits[] = { 0x40, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20 };

static const timediff_type daynight = 24*3600;

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
        if ( (activeWeekDays_ & weekBits[weekDay]) == 0 ) return false;
    }
    if ( activePeriodStart_ >= 0 ) {
        const timediff_type dayTime = weekTime % daynight;
        if ( activePeriodStart_ < activePeriodEnd_ ) {
            if (dayTime < activePeriodStart_ || dayTime >= activePeriodEnd_ ) return false;
        } else {
            if (dayTime < activePeriodStart_ && dayTime >= activePeriodEnd_ ) return false;
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
    // FIXME: update retry policy: data.retryPolicy
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
    /*
    if (startDate_ == 0) {
        startDate_ = currentTimeSeconds();
        char buf[30];
        const time_t tmp(startDate_);
        struct tm tmnow;
        gmtime_r(&tmp,&tmnow);
        sprintf(buf,"%02u.%02u.%04u %02u:%02u:%02u",
                tmnow.tm_mday, tmnow.tm_mon+1, tmnow.tm_year+1900,
                tmnow.tm_hour, tmnow.tm_min, tmnow.tm_sec);
        data_.startDate = buf;
    }
     */
}

}
}
