#include <cassert>
#include <cstring>
#include <vector>
#include "informer/io/InfosmeException.h"
#include "DeliveryInfo.h"
#include "CommonSettings.h"
#include "util/smstext.h"
#include "informer/io/UTF8.h"
#include "informer/data/MessageText.h"
#include "ActivityLog.h"
#include "UserInfo.h"
#include "informer/io/DirListing.h"
#include "core/buffers/TmpBuf.hpp"

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

const size_t DeliveryInfoData::NAME_LENGTH;
const size_t DeliveryInfoData::SVCTYPE_LENGTH;
const size_t DeliveryInfoData::USERDATA_LENGTH;

smsc::logger::Logger* DeliveryInfo::log_ = 0;

DeliveryInfo::DeliveryInfo( dlvid_type              dlvId,
                            const DeliveryInfoData& data,
                            UserInfo&               userInfo ) :
dlvId_(dlvId),
userInfo_(&userInfo),
startDate_(0),
endDate_(0),
activePeriodStart_(-1),
activePeriodEnd_(-1),
validityPeriod_(-1),
activeWeekDays_(-1)
{
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("dlvinfo");
    }
    updateData( data, 0 );

    // stats
    stats_.clear();
    incstats_[0].clear();
    incstats_[1].clear();
    readStats();
}


void DeliveryInfo::update( const DeliveryInfoData& data )
{
    updateData( data, &data_);
}


int DeliveryInfo::checkActiveTime( int weekTime ) const
{
    const unsigned weekDay = unsigned(weekTime / daynight);
    const timediff_type dayTime = weekTime % daynight;
    int res = 0;
    bool nextDay = false;
    MutexGuard mg(lock_);
    const char* what = "ok";
    do {

        if (activeWeekDays_ != -1) {
            if ( weekDay>= 7 ) {
                throw InfosmeException(EXC_LOGICERROR,"D=%u wrong weekTime=%d -> day=%u",dlvId_,weekTime,weekDay);
            }
            if ( (activeWeekDays_ & weekBits[weekDay]) == 0 ) {
                what = "bad day";
                res = daynight - dayTime;
                nextDay = true;
                break;
            }
        }

        if ( activePeriodStart_ >= 0 ) {
            if ( activePeriodStart_ < activePeriodEnd_ ) {
                if (dayTime < activePeriodStart_) {
                    what = "before start";
                    res = activePeriodStart_ - dayTime;
                    break;
                } else if ( dayTime >= activePeriodEnd_ ) {
                    what = "after end";
                    nextDay = true;
                    res = daynight - dayTime;
                    break;
                }
                res = dayTime - activePeriodEnd_;
            } else {
                if (dayTime < activePeriodEnd_) {
                    res = dayTime - activePeriodEnd_;
                } else if (dayTime < activePeriodStart_) {
                    what = "before invstart";
                    res = activePeriodStart_ - dayTime;
                    break;
                }
                res = dayTime - daynight;
                nextDay = true;
            }
        }
                
        // allowed, res<0, check nextDay if needed
        if (nextDay) {
            if ( (activeWeekDays_ & weekBits[(weekDay+1) % 7]) == 0 ) {
                // the whole next day forbidden
                what = "until nextday";
            } else {
                what = "until nextday end";
                res -= activePeriodEnd_;
            }
        }
            
    } while (false);

    if (res>0) {
        for ( unsigned i = weekDay+1; ; ++i ) {
            if ( i >= 7 ) { i -= 7; }
            if ( i == weekDay ) {
                throw InfosmeException(EXC_LOGICERROR,"D=%u wrong activeWeekDays=%u",dlvId_,activeWeekDays_);
            }
            if ((activeWeekDays_ & weekBits[i]) != 0) { break; }
            res += daynight; // the whole day forbidden
        }
        if ( activePeriodStart_ >= 0 && activePeriodStart_ < activePeriodEnd_ ) {
            res += activePeriodStart_;
        }
    }
    smsc_log_debug(log_,"D=%u checkActive(%d): day=%u/time=%d res=%d: %s",
                   dlvId_, weekTime, weekDay, dayTime, res, what);
    return res;
}


unsigned DeliveryInfo::evaluateNchunks( const char*     outText,
                                        size_t          outLen,
                                        smsc::sms::SMS* sms ) const
{
    try {
        const char* out = outText;
        UTF8::BufType ucstext;
        const bool hasHighBit = smsc::util::hasHighBit(out,outLen);
        if (hasHighBit) {
            getCS()->getUTF8().convertToUcs2(out,outLen,ucstext);
            outLen = ucstext.GetPos();
            out = ucstext.get();
            if (sms) sms->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, DataCoding::UCS2);
        } else if (sms) {
            sms->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, DataCoding::LATIN1);
        }

        if ( outLen <= MAX_ALLOWED_MESSAGE_LENGTH && !this->useDataSm() ) {
            if (sms) {
                sms->setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, out, unsigned(outLen));
                sms->setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH, unsigned(outLen));
            }
        } else if ( this->getDeliveryMode() != DLVMODE_SMS ) {
            // ussdpush*
            if ( outLen > MAX_ALLOWED_MESSAGE_LENGTH ) {
                smsc_log_warn(log_,"ussdpush: max allowed msg len reached: %u",unsigned(outLen));
                outLen = MAX_ALLOWED_MESSAGE_LENGTH;
            }
            if (sms) {
                sms->setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, out, unsigned(outLen));
                sms->setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH, unsigned(outLen));
            }
        } else {
            if ( outLen > MAX_ALLOWED_PAYLOAD_LENGTH ) {
                outLen = MAX_ALLOWED_PAYLOAD_LENGTH;
            }
            if (sms) {
                sms->setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, out, unsigned(outLen));
            }
        }
        
        const unsigned chunkLen = getCS()->getSlicedMessageSize();
        unsigned nchunks;
        if ( chunkLen > 0 && outLen > chunkLen ) {
            nchunks = unsigned(outLen-1) / chunkLen + 1;
        } else {
            nchunks = 1;
        }
        return nchunks;
    } catch ( std::exception& e ) {
        throw InfosmeException(EXC_IOERROR,"bad msg body: '%s'",outText);
    }
    return 1;
}


void DeliveryInfo::incMsgStats( uint8_t state, int value, uint8_t fromState, int smsValue )
{
    MutexGuard mg(statLock_);
    stats_.incStat(state,value,smsValue);
    if (fromState) {stats_.incStat(fromState,-value,0);}
    const unsigned idx = getCS()->getStatBankIndex();
    incstats_[idx].incStat(state,value,smsValue);
    // doIncStats(state,value,fromState,smsValue);
}


void DeliveryInfo::popMsgStats( DeliveryStats& ds )
{
    MutexGuard mg(statLock_);
    const unsigned idx = 1 - getCS()->getStatBankIndex();
    ds = incstats_[idx];
    incstats_[idx].clear();
}


void DeliveryInfo::updateData( const DeliveryInfoData& data,
                               const DeliveryInfoData* old )
{
    // pre-check
    if ( data.svcType.size() >= DeliveryInfoData::SVCTYPE_LENGTH ) {
        throw InfosmeException(EXC_BADNAME,"D=%u too long svcType '%s'",
                               dlvId_, data.svcType.c_str());
    }
    if ( data.name.size() >= DeliveryInfoData::NAME_LENGTH ) {
        throw InfosmeException(EXC_BADNAME,"D=%u too long name '%s'",
                               dlvId_, data.name.c_str());
    }
    if ( data.userData.size() >= DeliveryInfoData::USERDATA_LENGTH ) {
        throw InfosmeException(EXC_BADNAME,"D=%u too long userdata '%s'",
                               dlvId_, data.userData.c_str());
    }

    MutexGuard mg(lock_);
    msgtime_type startDate = startDate_;
    msgtime_type endDate = endDate_;
    timediff_type activePeriodStart = activePeriodStart_;
    timediff_type activePeriodEnd = activePeriodEnd_;
    timediff_type validityPeriod = validityPeriod_;
    int activeWeekDays = activeWeekDays_;
    smsc::sms::Address sourceAddress(sourceAddress_);
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
    bool sourceAddressChanged = false;
    if ((!old || old->sourceAddress != data.sourceAddress) && !data.sourceAddress.empty()) {
        // sourceAddress = parseAddress(data.sourceAddress.c_str());
        try {
            sourceAddress = smsc::sms::Address( data.sourceAddress.c_str() );
        } catch ( std::exception& e ) {
            throw InfosmeException(EXC_BADADDRESS,"invalid source address '%s': %s",data.sourceAddress.c_str(),e.what());
        }
        sourceAddressChanged = true;
    }

    // NOTE: we don't need this check as we don't use userdata in core
    // if ( !isGoodAsciiName(data.userData.c_str()) ) {
    // throw InfosmeException(EXC_BADNAME,"invalid chars in userData '%s'",data.userData.c_str());
    // }

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

    if ( sourceAddress.length <= 1 &&
         sourceAddress.type == 0 &&
         sourceAddress.plan == 0 ) {
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
    if (sourceAddressChanged) { sourceAddress_ = sourceAddress; }
    if (newRetryPolicy) { retryPolicy_ = retryPolicy; }
    data_ = data;
}


/// read the statistics
void DeliveryInfo::readStats()
{
    bool statsLoaded = false;
    try {
        DirListing< NoDotsNameFilter > dl( NoDotsNameFilter(), S_IFDIR );
        std::vector< std::string > dirs;
        char fnbuf[150];
        makeDeliveryPath(fnbuf,dlvId_);
        const std::string actpath = getCS()->getStorePath() + fnbuf + "activity_log/";
        dl.list( actpath.c_str(), dirs );
        std::sort( dirs.begin(), dirs.end() );
        std::vector< std::string > subdirs;
        subdirs.reserve(24);
        smsc::core::buffers::TmpBuf<char,8192> buf;
        for ( std::vector<std::string>::reverse_iterator i = dirs.rbegin();
              i != dirs.rend(); ++i ) {
            subdirs.clear();
            const std::string daypath = actpath + *i;
            dl.list( daypath.c_str(), subdirs );
            std::sort( subdirs.begin(), subdirs.end() );
            for ( std::vector<std::string>::reverse_iterator j = subdirs.rbegin();
                  j != subdirs.rend(); ++j ) {
                std::vector< std::string > logfiles;
                logfiles.reserve(60);
                const std::string hourpath = daypath + "/" + *j;
                makeDirListing( NoDotsNameFilter(), S_IFREG ).list( hourpath.c_str(), logfiles );
                std::sort(logfiles.begin(), logfiles.end());
                for ( std::vector< std::string >::reverse_iterator k = logfiles.rbegin();
                      k != logfiles.rend(); ++k ) {
                    
                    const std::string filename = hourpath + "/" + *k;
                    try {
                        if ( ActivityLog::readStatistics( filename,
                                                          buf,
                                                          stats_ ) ) {
                            statsLoaded = true;
                            break;
                        }
                    } catch ( std::exception& e ) {
                        smsc_log_warn(log_,"D=%u, file '%s' exc: %s",
                                      getDlvId(), filename.c_str(), e.what());
                    }
                }
                if (statsLoaded) break;
            }
            if (statsLoaded) break;
        }
    } catch (std::exception& e) {
        smsc_log_debug(log_,"D=%u stats, exc: %s", getDlvId(), e.what());
    }
    if (!statsLoaded) {
        smsc_log_debug(log_,"D=%u statistics is not found", getDlvId());
    }
}

}
}
