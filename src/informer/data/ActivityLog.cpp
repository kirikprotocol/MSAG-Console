#include <ctime>
#include "ActivityLog.h"
#include "CommonSettings.h"
#include "informer/io/TextEscaper.h"
#include "informer/io/DirListing.h"
#include "core/buffers/TmpBuf.hpp"
#include "UserInfo.h"
#include "FinalLog.h"

namespace {
const char* statsFormat = "# TOTAL=%u,PROC=%u,SENT=%u,RTRY=%u,DLVD=%u,FAIL=%u,EXPD=%u,SMSDLVD=%u,SMSFAIL=%u,SMSEXPD=%u,KILL=%u%n";
smsc::logger::Logger* log_ = 0;
}

namespace eyeline {
namespace informer {

ActivityLog::ActivityLog( UserInfo& userInfo,
                          DeliveryInfo* info ) :
userInfo_(&userInfo),
dlvInfo_(info),
createTime_(0)
{
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("actlog");
    }
    stats_.clear();
    incstats_[0].clear();
    incstats_[1].clear();
    period_ = 60;
    {
        if (period_ > 3600) period_ = 3600;
        const int nslices = 3600 / period_;
        period_ = 3600 / nslices;
        if (period_ < 60) period_ = 60;
    }
    // reading the last file in activity logs subdirs
    bool statsLoaded = false;
    try {
        DirListing< NoDotsNameFilter > dl( NoDotsNameFilter(), S_IFDIR );
        std::vector< std::string > dirs;
        char fnbuf[150];
        makeDeliveryPath(fnbuf,getDlvId());
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
                        if ( readStatistics( filename, buf ) ) {
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
        smsc_log_debug(log_,"D=%u actlog, exc: %s", getDlvId(), e.what());
    }
    if (!statsLoaded) {
        smsc_log_debug(log_,"D=%u statistics is not found", getDlvId());
    }
}


bool ActivityLog::readStatistics( const std::string& filename,
                                  smsc::core::buffers::TmpBuf<char, 8192>& buf )
{
    FileGuard fg;
    fg.ropen( filename.c_str() );
    buf.SetPos(0);
    bool statLineHasBeenRead = false;
    DeliveryStats ds;
    ds.clear();
    do {
        char* ptr = buf.get();
        const size_t wasread = fg.read( buf.GetCurPtr(), buf.getSize() - buf.GetPos() );
        if (wasread==0) {
            // EOF
            if (ptr < buf.GetCurPtr()) {
                // FIXME: should we truncate instead?
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
                // head comment, trying to read statline
                int shift = 0;
                sscanf(line, ::statsFormat,
                       &ds.totalMessages, &ds.procMessages,
                       &ds.sentMessages, &ds.retryMessages,
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
            case 'N' : ++ds.totalMessages;   break;
            case 'P' : ++ds.procMessages;    break;
            case 'R' : ++ds.retryMessages;   break;
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
                case 'E': ++ds.failedMessages; ds.failedSms += nchunks; break;
                case 'F': ++ds.expiredMessages; ds.expiredSms += nchunks; break;
                case 'K': ++ds.killedMessages; break;
                default: break;
                }
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
        stats_ = ds;
        // they will be supplied later
        stats_.sentMessages = stats_.procMessages = 0;
    }
    return statLineHasBeenRead;
}


void ActivityLog::addRecord( msgtime_type currentTime,
                             regionid_type regId,
                             const Message& msg,
                             int smppStatus,
                             uint8_t fromState )
{
    struct ::tm now;
    {
        const time_t tmnow(currentTime);
        if ( !gmtime_r(&tmnow,&now) ) {
            throw InfosmeException(EXC_SYSTEM,"D=%u gmtime_r",getDlvId());
        }
    }

    unsigned planTime = 0;
    char cstate = msgStateToString(MsgState(msg.state))[0];
    switch (msg.state) {
    case MSGSTATE_INPUT:
    case MSGSTATE_PROCESS:
    case MSGSTATE_DELIVERED:
    case MSGSTATE_FAILED:
    case MSGSTATE_EXPIRED:
    case MSGSTATE_KILLED:
        break;
    case MSGSTATE_RETRY:
        planTime = unsigned(msg.lastTime - currentTime);
        break;
    default: throw InfosmeException(EXC_LOGICERROR,"actlog unknown state %u",msg.state);
    }

    char caddr[30];
    printSubscriber(caddr,msg.subscriber);

    smsc::core::buffers::TmpBuf<char,1024> buf;
    const int off = sprintf(buf.get(), "%02u,%c,%u,%llu,%u,%u,%s,%d,%d,%s,\"",
                            unsigned(now.tm_sec), cstate, regId,
                            msg.msgId, msg.retryCount, planTime, 
                            caddr,
                            msg.timeLeft, smppStatus,
                            msg.userData.c_str());
    if ( off < 0 ) {
        throw InfosmeException(EXC_SYSTEM,"cannot printf to activity.log: %d",off);
    }
    buf.SetPos(off);
    escapeText(buf, msg.text.getText(),strlen(msg.text.getText()));
    buf.Append("\"\n",2);

    {
        MutexGuard mg(lock_);
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
        doIncStats(msg.state,1,fromState,msg.retryCount);
    }

    // writing final log
    if ( msg.state >= MSGSTATE_FINAL &&
         getDlvInfo().wantFinalMsgRecords() ) {
        FinalLog::getFinalLog()->addMsgRecord(currentTime,
                                              getDlvId(),
                                              getUserInfo().getUserId(),
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
        MutexGuard mg(lock_);
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
        const char* header = "#1 SEC,STATE,REGID,MSGID,RETRY/NSMS,PLAN,SUBSCRIBER,TTL,SMPP,USERDATA,TEXT\n";
        fg_.write(header,strlen(header));
        char headbuf[200];
        int headlen;
        {
            MutexGuard lmg(statLock_);
            int shift;
            headlen = sprintf(headbuf,
                              ::statsFormat,
                              stats_.totalMessages, stats_.procMessages,
                              stats_.sentMessages, stats_.retryMessages,
                              stats_.dlvdMessages, stats_.failedMessages,
                              stats_.expiredMessages, stats_.dlvdSms,
                              stats_.failedSms, stats_.expiredSms,
                              stats_.killedMessages,
                              &shift );
            if (headlen<0) {
                throw InfosmeException(EXC_SYSTEM,"cannot sprintf header");
            }
            headbuf[headlen++] = '\n';
        }
        fg_.write(headbuf,size_t(headlen));
    }
}


void ActivityLog::incStats( uint8_t state, int value, uint8_t fromState, int smsValue )
{
    MutexGuard mg(statLock_);
    doIncStats(state,value,fromState,smsValue);
}


void ActivityLog::doIncStats( uint8_t state, int value, uint8_t fromState, int smsValue )
{
    stats_.incStat(state,value,smsValue);
    if (fromState) {stats_.incStat(fromState,-value,0);}
    const unsigned idx = getCS()->getStatBankIndex();
    incstats_[idx].incStat(state,value,smsValue);
}


void ActivityLog::popIncrementalStats( DeliveryStats& ds )
{
    MutexGuard mg(statLock_);
    const unsigned idx = 1 - getCS()->getStatBankIndex();
    ds = incstats_[idx];
    incstats_[idx].clear();
}

}
}
