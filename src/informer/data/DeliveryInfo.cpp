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
#include "Region.h"
#include "informer/io/DirListing.h"
#include "informer/io/TmpBuf.h"

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

// const size_t DeliveryInfoData::NAME_LENGTH;
// const size_t DeliveryInfoData::SVCTYPE_LENGTH;
// const size_t DeliveryInfoData::USERDATA_LENGTH;

// smsc::logger::Logger* DeliveryInfo::log_ = 0;

DeliveryInfo::DeliveryInfo( dlvid_type              dlvId,
                            const DeliveryInfoData& data,
                            UserInfo&               userInfo ) :
log_(0),
dlvId_(dlvId),
userInfo_(&userInfo),
lock_( MTXWHEREAMI ),
startDate_(0),
endDate_(0),
creationDate_(0),
activePeriodStart_(-1),
activePeriodEnd_(-1),
validityPeriod_(-1),
messageTimeToLive_(-1),
archivationTime_(-1),
activeWeekDays_(-1),
statLock_( MTXWHEREAMI ),
isOldActLog_(false)
{
    char buf[30];
    sprintf(buf,"d.%03u",dlvId % 1000);
    log_ = smsc::logger::Logger::getInstance(buf);
    updateData( data, 0 );

    // stats
    stats_.clear();
    // incstats_[0].clear();
    // incstats_[1].clear();
    readStats();

    glossary_.init(dlvId);
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
    const char* what = "ok";
    {
        smsc::core::synchronization::MutexGuard mg(lock_ MTXWHEREPOST);
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

        if (res>0 && nextDay ) {
            // forbidden until the end of the day
            if ( (activeWeekDays_ & 0x7f) == 0 ) {
                throw InfosmeException(EXC_LOGICERROR,"D=%u wrong activeWeekDays=%u",dlvId_,activeWeekDays_);
            }
            for ( unsigned i = weekDay+1; ; ++i ) {
                if ( i >= 7 ) { i -= 7; }
                if ((activeWeekDays_ & weekBits[i]) != 0) {
                    // the first allowed day found
                    break;
                }
                res += daynight; // the whole day forbidden
            }
            if ( activePeriodStart_ >= 0 && activePeriodStart_ < activePeriodEnd_ ) {
                res += activePeriodStart_;
            }
        }
    }
    smsc_log_debug(log_,"D=%u checkActive(%d): day=%u/time=%d res=%d: %s",
                   dlvId_, weekTime, weekDay, dayTime, res, what);
    return res;
}


timediff_type DeliveryInfo::recalcTTL( int weekTime, timediff_type ttl, timediff_type uptonow ) const
{
    if ( ttl <= 0 ) {
        return ttl;
    }
    static const int weekLen = 7*24*3600;
    int npass = 0;
    while ( uptonow > 0 ) {
        int active = checkActiveTime(weekTime % weekLen);
        if ( ++npass > 20 ) {
            smsc_log_warn(log_,"D=%u checkExpired loop weekTime=%d ttl=%d uptonow=%d active=%d",
                          dlvId_, weekTime, ttl, uptonow, active );
            return -1;
        }
        if ( active > 0 ) {
            // not active, subtract from uptonow, but keep ttl
            uptonow -= active;
            weekTime += active;
            continue;
        }
        // active, subtract from both uptonow and ttl
        active = -active;
        if ( uptonow < active ) {
            active = uptonow;
        }
        uptonow -= active;
        ttl -= active;
        weekTime += active;
        if (ttl <= 0) {
            return ttl;
        }
    }
    return ttl;
}


unsigned DeliveryInfo::evaluateNchunks( const char*     outText,
                                        size_t          outLen,
                                        smsc::sms::SMS* sms ) const
{
    try {
        const char* out = outText;
        TmpBuf<char,1024> ucstext;
        const bool hasHighBit = smsc::util::hasHighBit(out,outLen);
        if (hasHighBit) {
            getCS()->getUTF8().convertToUcs2(out,outLen,ucstext);
            outLen = ucstext.GetPos();
            out = ucstext.get();
            if (sms) sms->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,
                                         smsc::util::DataCoding::UCS2);
        } else if (sms) {
            sms->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,
                                smsc::util::DataCoding::LATIN1);
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


void DeliveryInfo::initMsgStats( regionid_type,
                                 uint8_t state,
                                 int value )
{
    smsc::core::synchronization::MutexGuard mg(statLock_);
    stats_.incStat(state,value,0);
}


void DeliveryInfo::incMsgStats( const Region& region,
                                uint8_t state,
                                int value, uint8_t fromState, int smsValue )
{
    const regionid_type regionId = region.getRegionId();
    char smscId[SMSC_ID_LENGTH];
    region.getSmscId(smscId);
    smsc::core::synchronization::MutexGuard mg(statLock_);
    stats_.incStat(state,value,smsValue);
    if (fromState) {stats_.incStat(fromState,-value,0);}
    // aggregation stats
    const unsigned idx = getCS()->getStatBankIndex();
    // search for the position in the map
    StatList& sm = statlist_[idx];
    StatList::iterator iter = std::find(sm.begin(),sm.end(),regionId);
    IncStat* ptr;
    if ( iter == sm.end() ) {
        // not found
        iter = sm.insert(sm.begin(),new StatNode);
        ptr = &*iter;
        ptr->smscId = smscId;
        ptr->stats.clear();
        ptr->regionId = regionId;
        ptr->next = 0;
    } else {
        // looking for the correct smscId
        ptr = &*iter;
        do {
            if ( ptr->smscId == smscId ) { break; }
            if (!ptr->next) {
                // not found, insert new smscId
                ptr = (ptr->next = new IncStat);
                ptr->smscId = smscId;
                ptr->stats.clear();
                ptr->regionId = regionId;
                ptr->next = 0;
                break;
            }
            ptr = ptr->next;
        } while (true);
    }
    ptr->stats.incStat(state,value,smsValue);
}


bool DeliveryInfo::popMsgStats(IncStat& stats)
{
    if (stats.next != 0) {
        throw InfosmeException(EXC_LOGICERROR,"input incstat with next");
    }
    StatNode* ptr;
    {
        smsc::core::synchronization::MutexGuard mg(statLock_);
        const unsigned idx = 1 - getCS()->getStatBankIndex();
        StatList& sm = statlist_[idx];
        if ( sm.empty() ) return false;
        ptr = sm.front();
        if (!ptr) return false;
        sm.pop_front();
    }
    std::swap(stats.next,ptr->next);
    stats.smscId = ptr->smscId;
    stats.regionId = ptr->regionId;
    stats.stats = ptr->stats;
    delete ptr;
    return true;
}


msgtime_type DeliveryInfo::fixActLogFormat( msgtime_type currentTime )
{
    if ( !isOldActLog_ ) return 0;
    smsc_log_debug(log_,"D=%u fixing old activity log",getDlvId());
    ActivityLog al(this);
    const msgtime_type res = al.fixActLogFormat(currentTime);
    isOldActLog_ = false;
    return res;
}


ulonglong DeliveryInfo::fixCreationDate( dlvid_type dlvId )
{
    DirListing< NoDotsNameFilter > dl( NoDotsNameFilter(), S_IFDIR );
    std::vector< std::string > dirs;
    char fnbuf[150];
    makeDeliveryPath(fnbuf,dlvId);
    const std::string actpath = getCS()->getStorePath() + fnbuf + "activity_log/";
    try {
        dl.list( actpath.c_str(), dirs );
        std::sort( dirs.begin(), dirs.end() );
        std::vector< std::string > subdirs;
        subdirs.reserve(24);
        TmpBuf<char,8192> buf;
        for ( std::vector<std::string>::iterator i = dirs.begin();
              i != dirs.end(); ++i ) {
            // YYYY.MM.DD
            subdirs.clear();
            int pos = 0;
            unsigned year, month, mday;
            sscanf(i->c_str(),"%04u.%02u.%02u%n",&year, &month, &mday, &pos);
            if ( pos != int(i->size()) ) {
                continue;
            }
            const std::string daypath = actpath + *i;
            dl.list( daypath.c_str(), subdirs );
            std::sort( subdirs.begin(), subdirs.end() );
            for ( std::vector<std::string>::iterator j = subdirs.begin();
                  j != subdirs.end(); ++j ) {
                // HH
                pos = 0;
                unsigned hour;
                sscanf(j->c_str(),"%02u%n",&hour,&pos);
                if ( pos != int(j->size()) ) {
                    continue;
                }
                std::vector< std::string > logfiles;
                logfiles.reserve(60);
                const std::string hourpath = daypath + "/" + *j;
                makeDirListing( NoDotsNameFilter(), S_IFREG ).list( hourpath.c_str(), logfiles );
                std::sort(logfiles.begin(), logfiles.end());
                for ( std::vector< std::string >::iterator k = logfiles.begin();
                      k != logfiles.end(); ++k ) {
                    // MM.log
                    pos = 0;
                    unsigned min;
                    sscanf(k->c_str(),"%02u.log%n",&min,&pos);
                    if ( pos != int(k->size()) ) {
                        continue;
                    }
                    const std::string filename = hourpath + "/" + *k;
                    try {
                        unsigned sec;
                        if ( !ActivityLog::readFirstRecordSeconds(filename,buf,sec) ) continue;
                        return ((((ulonglong(year)*100+month)*
                                  100+mday)*100+hour)*100+min)*100+sec;
                    } catch (...) {
                        // ignore
                    }
                }
            }
        }
    } catch (...) {
    }
    return 0;
}


void DeliveryInfo::updateData( const DeliveryInfoData& data,
                               const DeliveryInfoData* old )
{
    // pre-check
    if ( data.svcType.size() >= DLV_SVCTYPE_LENGTH ) {
        throw InfosmeException(EXC_BADNAME,"D=%u too long svcType '%s'",
                               dlvId_, data.svcType.c_str());
    }
    if ( data.name.size() >= DLV_NAME_LENGTH ) {
        throw InfosmeException(EXC_BADNAME,"D=%u too long name '%s'",
                               dlvId_, data.name.c_str());
    }
    if ( data.userData.size() >= DLV_USERDATA_LENGTH ) {
        throw InfosmeException(EXC_BADNAME,"D=%u too long userdata '%s'",
                               dlvId_, data.userData.c_str());
    }

    smsc::core::synchronization::MutexGuard mg(lock_ MTXWHEREPOST);
    msgtime_type startDate = startDate_;
    msgtime_type endDate = endDate_;
    timediff_type activePeriodStart = activePeriodStart_;
    timediff_type activePeriodEnd = activePeriodEnd_;
    timediff_type validityPeriod = validityPeriod_;
    timediff_type messageTimeToLive = messageTimeToLive_;
    timediff_type archivationPeriod = archivationTime_;

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
        validityPeriod = parseTime(data.validityPeriod.c_str(), true);
    }
    if ((!old || old->messageTimeToLive != data.messageTimeToLive) && !data.messageTimeToLive.empty() ) {
        messageTimeToLive = parseTime(data.messageTimeToLive.c_str(), true);
    }
    if ((!old || old->archivationPeriod != data.archivationPeriod) && !data.archivationPeriod.empty() ) {
        archivationPeriod = parseTime(data.archivationPeriod.c_str(), true);
    }
    if (!old || ( old->retryPolicy != data.retryPolicy ||
                  old->retryOnFail != data.retryOnFail ) ) {
        if (!data.retryOnFail) {
            retryPolicy.init("");
        } else if (data.retryPolicy.empty()) {
            // should be replaced
            retryPolicy.init("1s:*");
        } else {
            retryPolicy.init(data.retryPolicy.c_str());
        }
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
    const msgtime_type now = currentTimeSeconds();
    if (startDate != 0) {
        startDate_ = startDate;
    } else {
        // treat it as now
        startDate_ = now;
    }
    if (endDate != 0) { endDate_ = endDate; }
    if (activePeriodStart != -1) { activePeriodStart_ = activePeriodStart; }
    if (activePeriodEnd != -1) { activePeriodEnd_ = activePeriodEnd; }
    if (validityPeriod != -1) {
        validityPeriod_ = validityPeriod;
    } else {
        validityPeriod_ = getCS()->getValidityPeriodDefault();
    }
    if (messageTimeToLive != -1) {
        messageTimeToLive_ = messageTimeToLive;
    } else {
        messageTimeToLive_ = getCS()->getMessageTimeToLiveDefault();
    }
    if (archivationPeriod != -1) { archivationTime_ = archivationPeriod; }
    if (activeWeekDays != -1) { activeWeekDays_ = activeWeekDays; }
    if (sourceAddressChanged) { sourceAddress_ = sourceAddress; }
    if (newRetryPolicy) { retryPolicy_ = retryPolicy; }
    std::string creationDate;
    if (!creationDate_) {
        if (old) {
            creationDate = old->creationDate;
        } else if ( !data.creationDate.empty() ) {
            creationDate = data.creationDate;
        } else {
            tm ltm;
            msgTimeToYmd(now,&ltm);
            char buf[30];
            sprintf(buf,"%02u.%02u.%04u %02u:%02u:%02u",
                    ltm.tm_mday, ltm.tm_mon+1, ltm.tm_year+1900,
                    ltm.tm_hour, ltm.tm_min, ltm.tm_sec );
            creationDate = buf;
        }
    } else {
        creationDate = data_.creationDate;
    }
    data_ = data;
    data_.creationDate = creationDate;
    if (!creationDate_) {
        creationDate_ = parseDateTime(creationDate.c_str());
    }
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
        TmpBuf<char,8192> buf;
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
                                                          stats_,
                                                          isOldActLog_ )) {
                            statsLoaded = true;
                            if (isOldActLog_) {
                                smsc_log_warn(log_,"D=%u has old activity_log, will be fixed later",
                                              getDlvId());
                            }
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
