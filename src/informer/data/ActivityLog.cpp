#include <ctime>
#include "ActivityLog.h"
#include "CommonSettings.h"
#include "informer/io/TextEscaper.h"
#include "informer/io/HexDump.h"
#include "UserInfo.h"
#include "FinalLog.h"
#include "Region.h"

namespace eyeline {
namespace informer {

namespace {
const char* statsFormat2 = "# TOTAL=%u,PROC=%u,SENT=%u,RTRY=%u,DLVD=%u,FAIL=%u,EXPD=%u,SMSDLVD=%u,SMSFAIL=%u,SMSEXPD=%u,KILL=%u%n";
const char* statsFormat3 = "# TOTAL=%u,PROC=%u,SENT=%u,NEW=%u,DLVD=%u,FAIL=%u,EXPD=%u,SMSDLVD=%u,SMSFAIL=%u,SMSEXPD=%u,KILL=%u%n";
}

ActivityLog::ActivityLog( DeliveryInfo* info ) :
lock_( MTXWHEREAMI ),
dlvInfo_(info),
createTime_(0)
{
    {
        char buf[10];
        sprintf(buf,"alog.%u",getDlvId() % 1000);
        log_ = smsc::logger::Logger::getInstance(buf);
    }

    period_ = 60;
    {
        if (period_ > 3600) period_ = 3600;
        const int nslices = 3600 / period_;
        period_ = 3600 / nslices;
        if (period_ < 60) period_ = 60;
    }
}


bool ActivityLog::readStatistics( const std::string& filename,
                                  TmpBufBase<char>& buf,
                                  DeliveryStats& ods )
{
    FileGuard fg;
    fg.ropen( filename.c_str() );
    buf.SetPos(0);
    bool statLineHasBeenRead = false;
    int version = 0;
    DeliveryStats ds;
    ds.clear();
    do {
        char* ptr = buf.get();
        const size_t wasread = fg.read( buf.GetCurPtr(), buf.getSize() - buf.GetPos() );
        if (wasread==0) {
            // EOF
            if (ptr < buf.GetCurPtr()) {
                // FIXME: the activity log is corrupted, should we truncate?
                throw InfosmeException(EXC_BADFILE,"file is not terminated with LF");
            }
            break;
        }
        buf.SetPos(buf.GetPos()+wasread);
        while ( ptr < buf.GetCurPtr() ) {
            char* end = const_cast<char*>
                (reinterpret_cast<const char*>
                    (memchr(ptr,'\n',buf.GetCurPtr()-ptr)));
            if (!end) break; // EOL not found yet
            
            // EOL found
            *end = '\0';
            const char* line = ptr;
            ptr = end+1;
            if ( *line == '#' ) {
                // head comment
                if ( version == 0 ) {
                    // reading the version
                    int shift = 0;
                    sscanf(line,"#%u SEC,%n",&version,&shift);
                    if (shift == 0 || version <= 0 || version > 3) {
                        throw InfosmeException(EXC_BADFILE,"version cannot be read/is not supported (%s)",line);
                    }
                    continue;
                }
                // trying to read statline, version must be already defined
                int shift = 0;
                sscanf(line, version == 2 ? statsFormat2 : statsFormat3,
                       &ds.totalMessages, &ds.procMessages,
                       &ds.sentMessages, &ds.newMessages,
                       &ds.dlvdMessages, &ds.failedMessages,
                       &ds.expiredMessages, &ds.dlvdSms,
                       &ds.failedSms, &ds.expiredSms,
                       &ds.killedMessages,
                       &shift );
                if ( version == 2 ) {
                    // we cannot recreate new messages count
                    ds.newMessages = 0;
                }
                if (shift==0) {
                    // not a stat line
                    continue;
                }
                if (statLineHasBeenRead) {
                    throw InfosmeException(EXC_BADFILE,"duplicate stat line");
                }
                statLineHasBeenRead = true;
                continue;

            }

            // record line, trying to read its head
            if (!statLineHasBeenRead) {
                throw InfosmeException(EXC_BADFILE,"no stat line before the first record");
            }
            int shift = 0;
            unsigned seconds;
            char cstate;
            sscanf(line,"%02u,%c,%n",&seconds,&cstate,&shift);
            if (!shift) {
                throw InfosmeException(EXC_BADFILE,"wrong record: '%s'",line);
            }
            switch (cstate) {
            case 'N' : ++ds.totalMessages; ++ds.newMessages;  break;
            case 'P' : /*++ds.procMessages;*/ --ds.newMessages;  break;
            case 'R' : /*++ds.retryMessages; */  break;
            case 'B' : break; // skip record - do nothing
            case 'D' :
            case 'E' :
            case 'F' :
            case 'K': {
                unsigned regId;
                ulonglong msgId;
                unsigned nchunks;
                int shift2 = 0;
                sscanf(line+shift,"%u,%llu,%u,%n",&regId,&msgId,&nchunks,&shift2);
                if (!shift2 || !nchunks) {
                    throw InfosmeException(EXC_BADFILE,"cannot parse the number of sms in '%s'",line);
                }
                switch (cstate) {
                case 'D': ++ds.dlvdMessages; ds.dlvdSms += nchunks; break;
                case 'F': ++ds.failedMessages; ds.failedSms += nchunks; break;
                case 'E': ++ds.expiredMessages; ds.expiredSms += nchunks; break;
                case 'K': ++ds.killedMessages; break;
                default: break;
                }
                break;
            }
            default:
                throw InfosmeException(EXC_BADFILE,"unknown record state in '%s'",line);
            }

        } // while there is LF in buffer

        if (ptr > buf.get()) {
            // shifting buffer
            char* o = buf.get();
            const char* i = ptr;
            const char* e = buf.GetCurPtr();
            for ( ; i < e; ) {
                *o++ = *i++;
            }
            buf.SetPos(o-buf.get());
        } else {
            // buffer is too small
            if (buf.getSize() > 100000) {
                throw InfosmeException(EXC_BADFILE,"too big record");
            }
            buf.reserve(buf.getSize()+buf.getSize()/2+100);
        }
    } while (true);
    if (statLineHasBeenRead) {
        ods = ds;
        ods.sentMessages = ods.procMessages = 0;
    }
    return statLineHasBeenRead;
}


void ActivityLog::addRecord( msgtime_type currentTime,
                             const Region& region,
                             const Message& msg,
                             int smppStatus,
                             uint8_t fromState )
{
    const regionid_type regId = region.getRegionId();
    struct ::tm now;
    {
        const time_t tmnow(currentTime);
        if ( !gmtime_r(&tmnow,&now) ) {
            throw InfosmeException(EXC_SYSTEM,"D=%u gmtime_r",getDlvId());
        }
    }

    unsigned planTime = 0;
    int retryCount = int(msg.retryCount);
    char cstate = msgStateToString(MsgState(msg.state))[0];
    switch (msg.state) {
    case MSGSTATE_INPUT:
    case MSGSTATE_PROCESS:
        break;
    case MSGSTATE_DELIVERED:
    case MSGSTATE_FAILED:
    case MSGSTATE_EXPIRED:
    case MSGSTATE_KILLED:
        if (!retryCount) {
            const char* text = msg.text.getText();
            if (!text) {
                throw InfosmeException(EXC_LOGICERROR,"R=%u/D=%u/M=%llu text is NULL",
                                       regId, getDlvId(), ulonglong(msg.msgId) );
            }
            retryCount = int(dlvInfo_->evaluateNchunks(text,strlen(text)));
        }
        break;
    case MSGSTATE_RETRY:
        planTime = unsigned(msg.lastTime - currentTime);
        break;
    default: throw InfosmeException(EXC_LOGICERROR,"actlog unknown state %u",msg.state);
    }

    char caddr[30];
    printSubscriber(caddr,msg.subscriber);

    TmpBuf<char,1024> buf;
    int off = sprintf(buf.get(), "%02u,%c,%u,%llu,%u,%u,%s,%d,%d,%s,",
                      unsigned(now.tm_sec), cstate, regId,
                      msg.msgId, retryCount, planTime, 
                      caddr,
                      msg.timeLeft, smppStatus,
                      msg.userData.c_str());
    if ( off < 0 ) {
        throw InfosmeException(EXC_SYSTEM,"cannot printf to activity.log: %d",off);
    }
    buf.SetPos(off);
    if ( ! msg.flags.isEmpty() ) {
        HexDump hd(false);
        off += int(hd.hexdumpsize(msg.flags.bufsize())) + 1;
        buf.reserve(off);
        hd.hexdump(buf.GetCurPtr(),msg.flags.buf(),msg.flags.bufsize());
        buf.SetPos(off);
    }
    buf.Append(",\"",2);
    escapeText(buf, msg.text.getText(),strlen(msg.text.getText()));
    buf.Append("\"\n",2);

    {
        smsc::core::synchronization::MutexGuard mg(lock_ MTXWHEREPOST);
        if ( !fg_.isOpened() || (createTime_+period_) <= currentTime ) {
            createFile(currentTime,now);
        }

        if ( currentTime < createTime_ ) {
            // a fix for delayed write
            smsc_log_debug(log_,"D=%u fix for delayed write, creaTime-curTime=%u",
                           getDlvId(), unsigned(createTime_ - currentTime));
            ::memcpy(buf.get(),"00",2);
        }
        fg_.write(buf.get(),buf.GetPos());
        dlvInfo_->incMsgStats(region,msg.state,1,fromState,retryCount);
    }

    // writing final log
    if ( msg.state >= MSGSTATE_FINAL &&
         dlvInfo_->wantFinalMsgRecords() ) {
        FinalLog::getFinalLog()->addMsgRecord(currentTime,
                                              getDlvId(),
                                              dlvInfo_->getUserInfo().getUserId(),
                                              msg,
                                              smppStatus);
    }
}


void ActivityLog::addDeleteRecords( msgtime_type currentTime,
                                    const std::vector<msgid_type>& msgIds )
{
    struct ::tm now;
    {
        const time_t tmnow(currentTime);
        if ( !gmtime_r(&tmnow,&now) ) {
            throw InfosmeException(EXC_SYSTEM,"D=%u gmtime_r",getDlvId());
        }
    }
    char buf[100];
    int shift = 0;
    sprintf(buf,"%02u,B,,%n",now.tm_sec,&shift); // banned
    if (!shift) {
        throw InfosmeException(EXC_SYSTEM,"cannot printf delrec");
    }
    {
        smsc::core::synchronization::MutexGuard mg(lock_ MTXWHEREPOST);
        if ( !fg_.isOpened() || (createTime_+period_) <= currentTime ) {
            createFile(currentTime,now);
        }

        if ( currentTime < createTime_ ) {
            // a fix for delayed write
            smsc_log_debug(log_,"D=%u fix for delayed write, creaTime-curTime=%u",
                           getDlvId(), unsigned(createTime_ - currentTime));
            ::memcpy(buf,"00",2);
        }
        for ( std::vector<msgid_type>::const_iterator i = msgIds.begin();
              i != msgIds.end(); ++i ) {
            const int off = sprintf(buf+shift,"%llu,,,,,,,\n",ulonglong(*i));
            fg_.write(buf,shift+off);
        }
    }
}


void ActivityLog::fsync()
{
    smsc::core::synchronization::MutexGuard mg(lock_ MTXWHEREPOST);
    fg_.fsync();
}


void ActivityLog::createFile( msgtime_type currentTime, struct tm& now )
{
    char fnbuf[100];
    const int oldmin = now.tm_min;
    now.tm_min = ((now.tm_min*60) / period_) * period_ / 60;
    sprintf(makeDeliveryPath(fnbuf,getDlvId()),
            "activity_log/%04u.%02u.%02u/%02u/%02u.log",
            now.tm_year+1900, now.tm_mon+1, now.tm_mday,
            now.tm_hour, now.tm_min );
    fg_.create((getCS()->getStorePath()+fnbuf).c_str(),
               0666, true );
    createTime_ = currentTime - (oldmin - now.tm_min)*60 - now.tm_sec;
    fg_.seek(0, SEEK_END);
    if (fg_.getPos() == 0) {
        const char* header = "#3 SEC,STATE,REGID,MSGID,RETRY/NSMS,PLAN,SUBSCRIBER,TTL,SMPP,USERDATA,FLAGS,TEXT\n";
        fg_.write(header,strlen(header));
        char headbuf[200];
        int headlen;
        {
            DeliveryStats ds;
            dlvInfo_->getMsgStats(ds);
            int shift;
            headlen = sprintf(headbuf,
                              statsFormat3,
                              ds.totalMessages, ds.procMessages,
                              ds.sentMessages, ds.newMessages,
                              ds.dlvdMessages, ds.failedMessages,
                              ds.expiredMessages, ds.dlvdSms,
                              ds.failedSms, ds.expiredSms,
                              ds.killedMessages,
                              &shift );
            if (headlen<0) {
                throw InfosmeException(EXC_SYSTEM,"cannot sprintf header");
            }
            headbuf[headlen++] = '\n';
        }
        fg_.write(headbuf,size_t(headlen));
    }
}

}
}
