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


DeliveryInfo* DeliveryInfo::makeDeliveryInfo( InfosmeCore& core,
                                              UserInfo&    userInfo,
                                              const DeliveryInfoData& data )
{
    // const dlvid_type dlvId = core.getNextDlvId();
    return 0;
}


DeliveryInfo* DeliveryInfo::readDeliveryInfo( InfosmeCore& core,
                                              dlvid_type   dlvId )
{
    std::auto_ptr< DeliveryInfo > ptr(new DeliveryInfo(core.getCS(),dlvId));
    ptr->read( core );
    return ptr.release();
}


void DeliveryInfo::setState( DlvState state, msgtime_type planTime )
{
    if (!userInfo_) {
        throw InfosmeException("D=%u userinfo is not set, cannot set state",dlvId_);
    }
    smsc_log_debug(log_,"D=%u changing state %s(%d) -> %s(%d), planTime=%u",
                   dlvId_,
                   dlvStateToString(DlvState(state_)), state_,
                   dlvStateToString(DlvState(state)), state,
                   planTime );
    userInfo_->incStats(cs_,state,state_);
    state_ = state;
    planTime_ = planTime;
}


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
        throw InfosmeException("U='%s' is not found",userId);
    }
    userInfo_ = user.get();
}


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

}
}
