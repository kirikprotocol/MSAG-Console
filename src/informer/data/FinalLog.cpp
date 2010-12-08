#include <cassert>
#include <string.h>
#include "informer/io/DirListing.h"
#include "FinalLog.h"
#include "Message.h"
#include "CommonSettings.h"

namespace {

using namespace eyeline::informer;

struct LogFileFilter
{
    LogFileFilter( std::vector<ulonglong>& logfiles ) :
    logfiles_(logfiles)
    {
    }

    bool operator() ( const char* fn ) const {
        int shift = 0;
        ulonglong ymd;
        sscanf(fn,"%llu.log%n",&ymd,&shift);
        if (!shift) return false;
        logfiles_.push_back(ymd);
        return false;
    }
private:
    std::vector<ulonglong>& logfiles_;
};

}


namespace eyeline {
namespace informer {

FinalLog* FinalLog::instance_ = 0;

FinalLog::FinalLog() :
createTime_(0),
period_(60)
{
    assert(!instance_);
    instance_ = this;

    // check if we have unrolled old files
    std::vector< ulonglong > logfiles;
    std::vector< std::string > dummy;
    ::LogFileFilter lff(logfiles);
    try {
        makeDirListing( lff, S_IFREG ).list( (getCS()->getStorePath() + "final_log").c_str(),
                                             dummy );
    } catch ( ErrnoException& e ) {
        return;
    }
    std::sort( logfiles.begin(), logfiles.end() );
    const ulonglong ymdTime = msgTimeToYmd(currentTimeSeconds());
    for ( std::vector< ulonglong >::const_iterator i = logfiles.begin();
          i != logfiles.end(); ++i ) {
        char fn[100];
        sprintf(fn,"final_log/%llu.log",*i);
        if ( *i < ymdTime ) {
            // need to roll
            rollFile(fn);
        } else if ( fg_.isOpened() ) {
            throw InfosmeException(EXC_LOGICERROR,"two open files in final_log");
        } else {
            createTime_ = ymdToMsgTime(*i);
            strcpy(filename_,fn);
            fg_.create(fn,0666);
        }
    }
}


FinalLog::~FinalLog()
{
    assert(instance_);
    instance_ = 0;
}


void FinalLog::addMsgRecord(msgtime_type         currentTime,
                            dlvid_type           dlvId,
                            const char*          userId,
                            const Message&       msg,
                            int                  smppStatus )
{
    char cstate;
    switch (msg.state) {
    case MSGSTATE_DELIVERED:
    case MSGSTATE_FAILED:
    case MSGSTATE_EXPIRED:
    case MSGSTATE_KILLED:
        cstate = msgStateToString(MsgState(msg.state))[0]; break;
    default: return;
    }
    char caddr[30];
    printSubscriber(caddr,msg.subscriber);
    char buf[200];
    const int bufsize = sprintf(buf,"%02u,%u,%s,0,%llu,%c,%d,%s,%s,%u\n",
                                currentTime % 60, dlvId, userId, msg.msgId,
                                cstate,
                                smppStatus, caddr, msg.userData.c_str(),
                                msg.retryCount );
    if (bufsize < 0) {
        throw InfosmeException(EXC_SYSTEM,"cannot printf to final.log: %d",bufsize);
    }
    MutexGuard mg(lock_);
    doCheckRollFile(currentTime,true);
    if ( currentTime < createTime_ ) {
        // fix for delayed write
        ::memcpy(buf,"00",2);
    }
    fg_.write(buf,bufsize);
}


void FinalLog::addDlvRecord( msgtime_type         currentTime,
                             dlvid_type           dlvId,
                             const char*          userId,
                             DlvState             state )
{
    int rectype;
    switch (state) {
    case DLVSTATE_ACTIVE : rectype = 1; break;
    case DLVSTATE_FINISHED : rectype = 2; break;
    default:
        return;
    }
    char buf[100];
    const int bufsize = sprintf(buf,"%02u,%u,%s,%d\n",
                                currentTime % 60, dlvId, userId, rectype );
    if (bufsize<0) {
        throw InfosmeException(EXC_SYSTEM, "cannot printf to final.log: %d",bufsize);
    }
    MutexGuard mg(lock_);
    doCheckRollFile(currentTime,true);
    if (currentTime < createTime_) {
        ::memcpy(buf,"00",2);
    }
    fg_.write(buf,bufsize);
}


void FinalLog::checkRollFile( msgtime_type currentTime )
{
    MutexGuard mg(lock_);
    doCheckRollFile(currentTime,false);
}


void FinalLog::doCheckRollFile( msgtime_type currentTime, bool create )
{
    if (fg_.isOpened() && (createTime_ + period_) <= currentTime ) {
        fg_.close();
        rollFile( filename_ );
    }

    if (!fg_.isOpened() && create) {

        struct ::tm now;
        const time_t t(currentTime);
        if (!gmtime_r(&t,&now)) {
            throw InfosmeException(EXC_SYSTEM,"checkRollTime: gmtime_r(%u) failed",currentTime);
        }

        const int oldmin = now.tm_min;
        now.tm_min = ((now.tm_min*60) / period_) * period_ / 60;
        sprintf(filename_,"final_log/%04u%02u%02u%02u%02u.log",
                now.tm_year+1900, now.tm_mon+1, now.tm_mday,
                now.tm_hour, now.tm_min );
        fg_.create((getCS()->getStorePath()+filename_).c_str(), 0666, true );
        createTime_ = currentTime - (oldmin - now.tm_min)*60 - now.tm_sec;
        fg_.seek(0, SEEK_END);
        if (fg_.getPos() == 0) {
            const char* header = "#1 SEC,DLVID,USERID,RECTYPE,MSGID,STATE,SMPP,SUBSCRIBER,USERDATA,NSMS\n";
            fg_.write(header,strlen(header));
        }
    }
}


void FinalLog::rollFile( const char* fn )
{
    char fnbuf[100];
    const size_t stemlen = ::strlen(fn) - 3;
    assert( stemlen < 90 );
    memcpy(fnbuf,fn,stemlen);
    memcpy(fnbuf+stemlen,"csv",4);
    if (-1 == rename((getCS()->getStorePath() + fn).c_str(),
                     (getCS()->getStorePath() + fnbuf).c_str())) {
        throw ErrnoException(errno,"rename('%s','%s')",fn,fnbuf);
    }
}

} // informer
} // smsc
