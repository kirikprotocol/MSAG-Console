#include <cassert>
#include <cstring>
#include <vector>
#include "informer/io/DirListing.h"
#include "informer/io/FileGuard.h"
#include "DeliveryInfo.h"
#include "CommonSettings.h"
#include "core/buffers/TmpBuf.hpp"
#include "UserInfo.h"
#include "InfosmeCore.h"
#include "util/smstext.h"

namespace eyeline {
namespace informer {

smsc::logger::Logger* DeliveryInfo::log_ = 0;


void DeliveryInfo::setState( DlvState state, msgtime_type planTime )
{
    smsc_log_debug(log_,"D=%u changing state %s(%d) -> %s(%d), planTime=%u",
                   dlvId_,
                   dlvStateToString(DlvState(state_)), state_,
                   dlvStateToString(DlvState(state)), state,
                   planTime );
    userInfo_.incStats(cs_,state,state_);
    state_ = state;
    planTime_ = planTime;
}


/*
void DeliveryInfo::read( InfosmeCore& core )
{
    smsc::core::buffers::TmpBuf<char,200> buf;
    const std::string& path = cs_.getStorePath();
    buf.setSize(path.size()+70);
    strcpy(buf.get(),path.c_str());
    assert(*(buf.get()+path.size()-1) == '/');
    // char* end = 
    makeDeliveryPath(dlvId_,buf.get()+path.size());
    smsc_log_debug(log_,"FIXME: reading D=%u info '%s'",dlvId_,buf.get());

    const char* userId = "bukind";
    UserInfoPtr user( core.getUserInfo( userId ) );
    if (!user.get()) {
        throw InfosmeException(EXC_NOTFOUND,"U='%s' is not found",userId);
    }
    userInfo_ = user.get();
}
 */


unsigned DeliveryInfo::evaluateNchunks( const char* out, size_t outLen ) const
{
    if ( smsc::util::hasHighBit(out,outLen) ) {
        // FIXME: replace with conversion from UTF8
        outLen *= 2;
    }
    if ( outLen <= MAX_ALLOWED_MESSAGE_LENGTH && !useDataSm() ) {
        // ok
    } else if ( getDeliveryMode() != DLVMODE_SMS ) {
        if (outLen > MAX_ALLOWED_MESSAGE_LENGTH) {
            outLen = MAX_ALLOWED_MESSAGE_LENGTH;
        }
    } else {
        if (outLen > MAX_ALLOWED_PAYLOAD_LENGTH) {
            outLen = MAX_ALLOWED_PAYLOAD_LENGTH;
        }
    }

    const unsigned chunkLen = cs_.getMaxMessageChunkSize();
    if (chunkLen>0 && outLen > chunkLen) {
        return unsigned(outLen-1)/chunkLen + 1;
    } else {
        return 1;
    }
}


DeliveryInfo* DeliveryInfo::readDeliveryInfo( InfosmeCore& core,
                                              dlvid_type   dlvId )
{
    char buf[100];
    makeDeliveryPath(dlvId,buf);
    const std::string path = core.getCS().getStorePath() + buf;

    DeliveryInfoData data;

    smsc_log_debug(log_,"FIXME: reading D=%u info '%s'",dlvId,path.c_str());
    const char* userId = "bukind";
    UserInfoPtr user( core.getUserInfo( userId ) );
    if (!user.get()) {
        throw InfosmeException(EXC_NOTFOUND,"U='%s' is not found",userId);
    }
    // userInfo_ = user.get();
    // FIXME: impl read delivery info
    // std::auto_ptr< DeliveryInfo > ptr(new DeliveryInfo(core.getCS(),dlvId));
    // ptr->read( core );
    // return ptr.release();

    data.name = "dummy";
    data.priority = 1;
    data.transactionMode = 0;
    data.startDate = "";
    data.endDate = "";
    data.activePeriodStart = "";
    data.activePeriodEnd = "";
    data.validityDate = "";
    data.validityPeriod = "01:00:00";
    data.flash = false;
    data.useDataSm = false;
    data.deliveryMode = DLVMODE_SMS;
    data.owner = userId;
    data.retryOnFail = true;
    data.retryPolicy = "fixme";
    data.replaceMessage = false;
    data.svcType = "info";
    data.userData = "dlv05";
    data.sourceAddress = ".0.1.10000";

    DeliveryInfo* info = new DeliveryInfo( core.getCS(),
                                           dlvId,
                                           data,
                                           * user.get() );
    info->setState(DLVSTATE_PAUSED,0);
    return info;
}


DeliveryInfo::DeliveryInfo( const CommonSettings&   cs,
                            dlvid_type              dlvId,
                            const DeliveryInfoData& data,
                            UserInfo&               userInfo ) :
cs_(cs),
userInfo_(userInfo),
dlvId_(dlvId),
state_(DlvState(0)),
planTime_(0),
startDate_(0),
endDate_(0),
activePeriodStart_(-1),
activePeriodEnd_(-1),
validityDate_(0),
validityPeriod_(-1),
activeWeekDays_(-1),
sourceAddress_(0)
{
    updateData( data, 0 );
}


void DeliveryInfo::updateData( const DeliveryInfoData& data,
                               const DeliveryInfoData* old )
{
    msgtime_type startDate = startDate_;
    msgtime_type endDate = endDate_;
    timediff_type activePeriodStart = activePeriodStart_;
    timediff_type activePeriodEnd = activePeriodEnd_;
    msgtime_type validityDate = validityDate_;
    timediff_type validityPeriod = validityPeriod_;
    int activeWeekDays = activeWeekDays_;
    personid_type sourceAddress = sourceAddress_;

    if (!old && !data.startDate.empty()) { // calculate only at start
        startDate = parseDateTime(data.startDate);
    }
    if (!old || old->endDate != data.endDate ) {
        endDate = parseDateTime(data.endDate);
    }
    if (!old || old->activePeriodStart != data.activePeriodStart ) {
        activePeriodStart = parseTime(data.activePeriodStart);
    }
    if (!old || old->activePeriodEnd != data.activePeriodEnd ) {
        activePeriodEnd = parseTime(data.activePeriodEnd);
    }
    if (!old || old->activeWeekDays != data.activeWeekDays ) {
        activeWeekDays = parseWeekDays(data.activeWeekDays);
    }
    if (!old || old->validityDate != data.validityDate ) {
        validityDate = parseDateTime(data.validityDate);
    }
    if (!old || old->validityPeriod != data.validityPeriod) {
        validityPeriod = parseTime(data.validityPeriod);
    }
    // FIXME: update retry policy: data.retryPolicy
    if (!old || old->sourceAddress != data.sourceAddress) {
        sourceAddress = parseAddress(data.sourceAddress);
    }

    if ( !isGoodAsciiName(data.userData.c_str()) ) {
        throw InfosmeException(EXC_BADNAME,"invalid chars in userData '%s'",data.userData);
    }

    /// post-parsing check & fill
    if ( ( activePeriodStart < 0 && activePeriodEnd >= 0 ) ||
         ( activePeriodStart >= 0 && activePeriodEnd < 0 ) ) {
        throw InfosmeException(EXC_CONFIG,"invalid active period start/end");
    }

    if ( validityDate <= 0 && validityPeriod <= 0 ) {
        throw InfosmeException(EXC_CONFIG,"invalid validity date/period");
    }

    if ( sourceAddress == 0 ) {
        throw InfosmeException(EXC_CONFIG,"source address in not specified");
    }

    // filling
    if (startDate != 0) { startDate_ = startDate; }
    if (endDate != 0) { endDate_ = endDate; }
    if (activePeriodStart != -1) { activePeriodStart_ = activePeriodStart; }
    if (activePeriodEnd != -1) { activePeriodEnd_ = activePeriodEnd; }
    if (validityDate != 0) { validityDate_ = validityDate; }
    if (validityPeriod != -1) { validityPeriod_ = validityPeriod; }
    if (activeWeekDays != -1) { activeWeekDays_ = activeWeekDays; }
    if (sourceAddress != 0) { sourceAddress_ = sourceAddress; }
    data_ = data;
}

}
}
