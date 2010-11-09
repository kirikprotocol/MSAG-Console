#include <ctime>
#include "ActivityLog.h"
#include "DeliveryInfo.h"
#include "CommonSettings.h"
#include "informer/io/TextEscaper.h"
#include "informer/io/DirListing.h"
#include "core/buffers/TmpBuf.hpp"

namespace {
const char* statsFormat = "# TOTAL=%u,PROC=%u,SENT=%u,RTRY=%u,DLVD=%u,FAIL=%u,EXPD=%u,SMSDLVD=%u,SMSFAIL=%u,SMSEXPD=%u%n";
smsc::logger::Logger* log_ = 0;
}

namespace eyeline {
namespace informer {

ActivityLog::ActivityLog( const DeliveryInfo& info ) :
info_(info),
createTime_(0)
{
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("actlog");
    }
    stats_.clear();
    incstats_[0].clear();
    incstats_[1].clear();
    period_ = info.getCS().getActivityLogPeriod() / 60 * 60;
    if (period_ > 3600) period_ = 3600;
    const int nslices = 3600 / period_;
    period_ = 3600 / nslices;
    if (period_ < 60) period_ = 60;
    // reading the last file in activity logs subdirs
    bool statsLoaded = false;
    try {
        DirListing< NoDotsNameFilter > dl( NoDotsNameFilter(), S_IFDIR );
        std::vector< std::string > dirs;
        char fnbuf[150];
        makeDeliveryPath(info.getDlvId(),fnbuf);
        const std::string actpath = info.getCS().getStorePath() + fnbuf + "activity_log/";
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
                                      info_.getDlvId(), filename.c_str(), e.what());
                    }
                }
                if (statsLoaded) break;
            }
            if (statsLoaded) break;
        }
    } catch (std::exception& e) {
        smsc_log_debug(log_,"D=%u actlog: %s", info_.getDlvId(), e.what());
    }
    if (!statsLoaded) {
        smsc_log_warn(log_,"D=%u statistics is not found", info_.getDlvId());
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
            case 'D' :
            case 'E' :
            case 'F' : {
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


dlvid_type ActivityLog::getDlvId() const {
    return info_.getDlvId();
}


void ActivityLog::addRecord( msgtime_type currentTime,
                             regionid_type regId,
                             const Message& msg,
                             int smppStatus,
                             uint8_t fromState )
{
    struct ::tm now;
    const ulonglong ymdTime = msgTimeToYmd(currentTime,&now);

    unsigned planTime = 0;
    char cstate;
    switch (msg.state) {
    case MSGSTATE_INPUT:     cstate = 'N'; break;
    case MSGSTATE_PROCESS:   cstate = 'P'; break;
    case MSGSTATE_RETRY:
        planTime = unsigned(msg.lastTime - currentTime);
        break;
    case MSGSTATE_DELIVERED: cstate = 'D'; break;
    case MSGSTATE_FAILED:    cstate = 'F'; break;
    case MSGSTATE_EXPIRED:   cstate = 'E'; break;
    default: throw InfosmeException(EXC_LOGICERROR,"actlog unknown state %u",msg.state);
    }

    uint8_t ton, npi, len;
    const uint64_t addr = subscriberToAddress(msg.subscriber,len,ton,npi);
    char caddr[30];
    if (npi == 1 && ton == 1) {sprintf(caddr,"+%0*.*llu",len,len,addr);}
    else if (npi == 1 && ton==0) {sprintf(caddr,"%0*.*llu",len,len,addr);}
    else { sprintf(caddr,".%u.%u.%0*.*llu",ton,npi,len,len,addr); }

    smsc::core::buffers::TmpBuf<char,1024> buf;
    const int off = sprintf(buf.get(), "%02u,%c,%u,%llu,%u,%u,%s,%d,%d,%s,\"",
                            unsigned(ymdTime % 100), cstate, regId,
                            msg.msgId, msg.retryCount, planTime, 
                            caddr,
                            msg.timeLeft, smppStatus,
                            msg.userData.c_str());
    if ( off < 0 ) {
        throw InfosmeException(EXC_SYSTEM,"cannot printf to activity.log: %d",off);
    }
    buf.SetPos(off);
    escapeText(buf, msg.text->getText(), strlen(msg.text->getText()));
    buf.Append("\"\n",2);

    {
        MutexGuard mg(lock_);
        if ( !fg_.isOpened() || (createTime_ + period_) < currentTime ) {
            char fnbuf[100];
            const int oldmin = now.tm_min;
            now.tm_min = ((now.tm_min*60) / period_) * period_ / 60;
            sprintf(makeDeliveryPath(info_.getDlvId(),fnbuf),
                    "activity_log/%04u.%02u.%02u/%02u/%02u.log",
                    now.tm_year+1900, now.tm_mon+1, now.tm_mday,
                    now.tm_hour, now.tm_min );
            fg_.create((info_.getCS().getStorePath()+fnbuf).c_str(),
                       0666, true );
            createTime_ = currentTime - (oldmin - now.tm_min)*60 - now.tm_sec;
            fg_.seek(0, SEEK_END);
            if (fg_.getPos() == 0) {
                const char* header = "#1 SEC,STATE,REGID,MSGID,RETRY,PLAN,SUBSCRIBER,TTL,SMPP,USERDATA,TEXT\n";
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
                                      &shift );
                    if (headlen<0) {
                        throw InfosmeException(EXC_SYSTEM,"cannot sprintf header");
                    }
                    headbuf[headlen++] = '\n';
                }
                fg_.write(headbuf,size_t(headlen));
            }
        }

        if ( currentTime < createTime_ ) {
            // a fix for delayed write
            smsc_log_debug(log_,"D=%u fix for delayed write, creaTime-curTime=%u",
                           info_.getDlvId(), unsigned(createTime_ - currentTime));
            ::memcpy(buf.get(),"00",2);
        }
        fg_.write(buf.get(),buf.GetPos());
        doIncStats(msg.state,1,fromState,msg.retryCount);
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
    const unsigned idx = info_.getCS().getStatBankIndex();
    incstats_[idx].incStat(state,value,smsValue);
}


void ActivityLog::popIncrementalStats( DeliveryStats& ds )
{
    MutexGuard mg(statLock_);
    const unsigned idx = 1 - info_.getCS().getStatBankIndex();
    ds = incstats_[idx];
    incstats_[idx].clear();
}

}
}
