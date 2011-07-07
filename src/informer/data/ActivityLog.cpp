#include <ctime>
#include "ActivityLog.h"
#include "CommonSettings.h"
#include "informer/io/TextEscaper.h"
#include "informer/io/HexDump.h"
#include "informer/snmp/SnmpManager.h"
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
                                  DeliveryStats& ods,
                                  bool& isOldVersion )
{
    FileGuard fg;
    fg.ropen( filename.c_str() );
    buf.SetPos(0);
    bool statLineHasBeenRead = false;
    int version = 0;
    DeliveryStats ds;
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
                    int v;
                    sscanf(line,"#%u SEC,%n",&v,&shift);
                    if (shift == 0 || v <= 0 || v > 3) {
                        throw InfosmeException(EXC_BADFILE,"version cannot be read/is not supported (%s)",line);
                    }
                    version = v;
                    if ( version <= 2 ) {
                        isOldVersion = true;
                    }
                    continue;
                }
                // trying to read statline, version must be already defined
                int shift = 0;
                ds.clear();
                sscanf(line, version == 2 ? statsFormat2 : statsFormat3,
                       &ds.totalMessages, &ds.procMessages,
                       &ds.sentMessages, &ds.newMessages,
                       &ds.dlvdMessages, &ds.failedMessages,
                       &ds.expiredMessages, &ds.dlvdSms,
                       &ds.failedSms, &ds.expiredSms,
                       &ds.killedMessages,
                       &shift );
                if (shift==0) {
                    // not a stat line
                    continue;
                }
                if (statLineHasBeenRead) {
                    throw InfosmeException(EXC_BADFILE,"duplicate stat line");
                }
                if (version<=2) {
                    // swapping new and retry messages
                    ds.newMessages = ds.calcRetryMessagesCount();
                }
                statLineHasBeenRead = true;
                ods = ds;
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
            case 'N' : ++ods.totalMessages; ++ods.newMessages;  break;
            case 'P' : /*++ods.procMessages;*/ --ods.newMessages;  break;
            case 'R' : /*++ods.retryMessages; */  break;
            case 'S' : break;
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
                case 'D': ++ods.dlvdMessages; ods.dlvdSms += nchunks; break;
                case 'F': ++ods.failedMessages; ods.failedSms += nchunks; break;
                case 'E': ++ods.expiredMessages; ods.expiredSms += nchunks; break;
                case 'K': ++ods.killedMessages; break;
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
        // they are to be set from StoreJournal
        ods.sentMessages = ods.procMessages = 0;
        // fixing negative newMessages
        if ( int(ods.newMessages) < 0 ) {
            ods.newMessages = 0;
        }
    }
    return statLineHasBeenRead;
}


bool ActivityLog::readFirstRecordSeconds( const std::string& filename,
                                          TmpBufBase<char>& buf,
                                          unsigned& seconds )
{
    FileGuard fg;
    fg.ropen( filename.c_str() );
    buf.SetPos(0);
    do {
        char* ptr = buf.get();
        const size_t wasread = fg.read( buf.GetCurPtr(), buf.getSize() - buf.GetPos() );
        if (wasread ==0) {
            // EOF
            if (ptr < buf.GetCurPtr()) {
                throw InfosmeException(EXC_BADFILE,"file '%s' is not terminated with LF",filename.c_str());
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
                // comment, skip it
                continue;
            }

            // record line
            int shift = 0;
            char cstate;
            sscanf(line,"%02u,%c,%n",&seconds,&cstate,&shift);
            if (!shift) {
                throw InfosmeException(EXC_BADFILE,"wrong record: '%s'",line);
            }
            return true;

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
                throw InfosmeException(EXC_BADFILE,"too big record in '%s'",filename.c_str());
            }
            buf.reserve(buf.getSize()+buf.getSize()/2+100);
        }
    } while (true);
    return false;
}


void ActivityLog::addRecord( msgtime_type currentTime,
                             const Region& region,
                             const Message& msg,
                             int     smppStatus,
                             int     nchunks,
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
    if (!nchunks && msg.state != MSGSTATE_PROCESS) {
        // calculate number of sms
        const char* text = msg.text.getText();
        if (!text) {
            smsc_log_warn(log_,"R=%u/D=%u/M=%llu text is NULL, statistics on SMS will be wrong possibly",
                          regId, getDlvId(), ulonglong(msg.msgId) );
            nchunks = 1;
        } else {
            nchunks = int(dlvInfo_->evaluateNchunks(text,strlen(text)));
        }
    }
    switch (msg.state) {
    case MSGSTATE_INPUT:
    case MSGSTATE_PROCESS:
    case MSGSTATE_SENT:
        break;
    case MSGSTATE_DELIVERED:
    case MSGSTATE_FAILED:
    case MSGSTATE_EXPIRED:
    case MSGSTATE_KILLED:
        // all final states write nchunks instead of retryCount!
        retryCount = nchunks;
        break;
    case MSGSTATE_RETRY:
        planTime = unsigned(msg.lastTime - currentTime);
        break;
    default: throw InfosmeException(EXC_LOGICERROR,"actlog unknown state %u",msg.state);
    }

    char caddr[30];
    printSubscriber(caddr,msg.subscriber);

    TmpBuf<char,1024> buf;
    int off = sprintf(buf.get(), "%02u,%c,%u,%llu,%u,%u,%s,%d,%d,",
                      unsigned(now.tm_sec), cstate, regId,
                      msg.msgId, retryCount, planTime, 
                      caddr,
                      msg.timeLeft, smppStatus);
    if ( off < 0 ) {
        throw InfosmeException(EXC_SYSTEM,"cannot printf to activity.log: %d",off);
    }
    buf.setPos(off);
    buf.Append(msg.msgUserData.c_str(),msg.msgUserData.size());
    buf.Append(",",1);
    off = int(buf.getPos());
    if ( ! msg.flags.isEmpty() ) {
        HexDump hd(false);
        off += int(hd.hexdumpsize(msg.flags.bufsize())) + 1;
        buf.reserve(off);
        hd.hexdump(buf.GetCurPtr(),msg.flags.buf(),msg.flags.bufsize());
        buf.setPos(off);
    }
    buf.Append(",\"",2);
    escapeText(buf, msg.text.getText(),strlen(msg.text.getText()));
    buf.Append("\"\n",2);

    try {

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
            dlvInfo_->incMsgStats(region,msg.state,1,fromState,nchunks);
        }

        // writing final log
        if ( msg.state >= MSGSTATE_FINAL &&
             dlvInfo_->wantFinalMsgRecords() ) {
            FinalLog::getFinalLog()->addMsgRecord(currentTime,
                                                  getDlvId(),
                                                  dlvInfo_->getUserInfo().getUserId(),
                                                  msg,
                                                  smppStatus,
                                                  nchunks );
        }
    } catch ( FileWriteException& e ) {
        if ( getCS()->getSnmp() ) {
            getCS()->getSnmp()->sendTrap( SnmpTrap::TYPE_FILEIO,
                                          SnmpTrap::SEV_MAJOR,
                                          "actlog",
                                          e.getFileName(),
                                          e.what() );
        }
        throw;
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
    try {
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
    } catch ( FileWriteException& e ) {
        if ( getCS()->getSnmp() ) {
            getCS()->getSnmp()->sendTrap( SnmpTrap::TYPE_FILEIO,
                                          SnmpTrap::SEV_MAJOR,
                                          "actlog",
                                          e.getFileName(),
                                          e.what() );
        }
        throw;
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
    makeActLogPath(fnbuf,now);
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


void ActivityLog::makeActLogPath( char* fnbuf, const struct tm& now ) const
{
    const int tm_min = ((now.tm_min*60) / period_) * period_ / 60;
    sprintf(makeDeliveryPath(fnbuf,getDlvId()),
            "activity_log/%04u.%02u.%02u/%02u/%02u.log",
            now.tm_year+1900, now.tm_mon+1, now.tm_mday,
            now.tm_hour, tm_min );
}


msgtime_type ActivityLog::fixActLogFormat( msgtime_type currentTime )
{
    currentTime += period_;
    const time_t tmnow(currentTime);
    struct ::tm now;
    if ( !gmtime_r(&tmnow,&now) ) {
        throw InfosmeException(EXC_SYSTEM,"D=%u gmtime_r",getDlvId());
    }

    smsc::core::synchronization::MutexGuard mg(lock_);
    createFile(currentTime, now);
    return createTime_;
}

}
}
