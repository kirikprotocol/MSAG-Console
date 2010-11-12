#include "FinalLog.h"
#include "Message.h"

namespace eyeline {
namespace informer {

FinalLog::FinalLog( const CommonSettings& cs ) :
cs_(cs),
createTime_(0),
period_(60)
{
}


void FinalLog::addMsgRecord(msgtime_type         currentTime,
                            dlvid_type           dlvId,
                            const char*          userId,
                            const Message&       msg,
                            int                  smppStatus )
{
    struct ::tm now;
    const ulonglong ymdTime = msgTimeToYmd(currentTime,&now);
    char caddr[30];
    printSubscriber(caddr,msg.subscriber);
    char buf[200];
    const int bufsize = sprintf(buf,"%02u,%u,%s,0,%llu,%d,%s,%s\n",
                                unsigned(ymdTime % 100), dlvId, userId, msg.msgId,
                                smppStatus, caddr, msg.userData.c_str() );
    if (bufsize < 0) {
        throw InfosmeException(EXC_SYSTEM,"cannot printf to final.log: %d",bufsize);
    }
    MutexGuard mg(lock_);
    checkRollFile(currentTime,now);
    if ( currentTime < createTime_ ) {
        // fix for delayed write
        ::memcpy(buf,"00",2);
    }
    fg_.write(buf,bufsize);
}


void FinalLog::checkRollFile( msgtime_type currentTime, struct ::tm& now )
{
    if (!fg_.isOpened() || (createTime_ + period_) < currentTime) {
        char fnbuf[100];
        const int oldmin = now.tm_min;
        now.tm_min = ((now.tm_min*60) / period_) * period_ / 60;
        sprintf(fnbuf,"final_log/%04u%02u%02u%02u%02u.csv",
                now.tm_year+1900, now.tm_mon+1, now.tm_mday,
                now.tm_hour, now.tm_min );
        fg_.create((cs_.getStorePath()+fnbuf).c_str(), 0666, true );
        createTime_ = currentTime - (oldmin - now.tm_min)*60 - now.tm_sec;
        fg_.seek(0, SEEK_END);
        if (fg_.getPos() == 0) {
            const char* header = "#1 SEC,DLVID,USERID,RECTYPE,MSGID,STATE,SMPP,SUBSCRIBER,USERDATA\n";
            fg_.write(header,strlen(header));
        }
    }
}


void FinalLog::addDlvRecord( msgtime_type         currentTime,
                             dlvid_type           dlvId,
                             const char*          userId,
                             DlvState             state )
{
}

} // informer
} // smsc
