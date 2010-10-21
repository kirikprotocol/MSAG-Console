#include "ActivityLog.h"
#include "DeliveryInfo.h"
#include "CommonSettings.h"
#include "informer/io/TextEscaper.h"
#include "core/buffers/TmpBuf.hpp"

namespace eyeline {
namespace informer {

ActivityLog::ActivityLog( const DeliveryInfo& info ) :
info_(info),
createTime_(0)
{
    stats_.clear();
    period_ = info.getCS().getActivityLogPeriod() / 60 * 60;
    if (period_ > 3600) period_ = 3600;
    const int nslices = 3600 / period_;
    period_ = 3600 / nslices;
    if (period_ < 60) period_ = 60;
}


dlvid_type ActivityLog::getDlvId() const {
    return info_.getDlvId();
}


void ActivityLog::addRecord( msgtime_type currentTime,
                             regionid_type regId,
                             const Message& msg,
                             int smppStatus,
                             uint8_t fromState )
{
    struct tm now;
    char timebuf[20];
    formatMsgTime(timebuf,currentTime,&now);

    char cstate;
    switch (msg.state) {
    case MSGSTATE_INPUT:     cstate = 'N'; break;
    case MSGSTATE_DELIVERED: cstate = 'D'; break;
    case MSGSTATE_FAILED:    cstate = 'F'; break;
    case MSGSTATE_EXPIRED:   cstate = 'E'; break;
    default: throw InfosmeException("unknown state %u",msg.state);
    }

    uint8_t ton, npi, len;
    const uint64_t addr = subscriberToAddress(msg.subscriber,len,ton,npi);
    char caddr[30];
    if (npi == 1 && ton == 1) {sprintf(caddr,"+%0*.*llu",len,len,addr);}
    else if (npi == 1 && ton==0) {sprintf(caddr,"%0*.*llu",len,len,addr);}
    else { sprintf(caddr,".%u.%u.%0*.*llu",ton,npi,len,len,addr); }

    smsc::core::buffers::TmpBuf<char,1024> buf;
    const int off = sprintf(buf.get(), "%s,%c,%u,%llu,%s,%d,%d,\"%s\",\"",
                            timebuf, cstate, regId, msg.msgId, caddr,
                            msg.timeLeft, smppStatus,
                            msg.userData.c_str());
    if ( off < 0 ) {
        throw InfosmeException("cannot printf to activity.log: %d",off);
    }
    buf.SetPos(off);
    escapeText(buf, msg.text->getText(), strlen(msg.text->getText()));
    buf.Append("\"\n",2);

    {
        MutexGuard mg(lock_);
        if ( !fg_.isOpened() || (createTime_ + period_) < currentTime ) {
            char buf[100];
            const int oldmin = now.tm_min;
            now.tm_min = ((now.tm_min*60) / period_) * period_ / 60;
            sprintf(makeDeliveryPath(info_.getDlvId(),buf),
                    "activity/%04u.%02u.%02u/%02u:%02u.log",
                    now.tm_year+1900, now.tm_mon+1, now.tm_mday,
                    now.tm_hour, now.tm_min );
            fg_.create((info_.getCS().getStorePath()+buf).c_str(),
                       true );
            createTime_ = currentTime - (oldmin - now.tm_min)*60 - now.tm_sec;
            fg_.seek(0, SEEK_END);
            if (fg_.getPos() == 0) {
                const char* header = "#1 TIME,STATE,REGID,MSGID,SUBSCRIBER,TTL,SMPP,FS,LS,TOTAL,SENT,DLVD,FAILD,EXPRD,USERDATA,TEXT\n";
                fg_.write(header,strlen(header));
                char headbuf[200];
                {
                    MutexGuard lmg(statLock_);
                    const int len = 
                        sprintf(headbuf,
                                "# TOTAL,PROC,SENT,RTRY,DLVD,FAIL,EXPR\n"
                                "# %u,%u,%u,%u,%u,%u,%u\n",
                                stats_.totalMessages, stats_.procMessages,
                                stats_.sentMessages, stats_.retryMessages,
                                stats_.dlvdMessages, stats_.failedMessages,
                                stats_.expiredMessages );
                    if (len<0) {
                        throw InfosmeException("cannot sprintf header");
                    }
                }
                fg_.write(headbuf,size_t(len));
            }
        }
        fg_.write(buf.get(),buf.GetPos());
    }
    incStats(msg.state,1,fromState);
}


void ActivityLog::incStats( uint8_t state, int value, uint8_t fromState )
{
    MutexGuard mg(statLock_);
    stats_.incStat(state,value);
    if (fromState) {stats_.incStat(fromState,-value);}
}


}
}
