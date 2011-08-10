#include <cassert>
#include <string.h>
#include "informer/io/DirListing.h"
#include "informer/io/TmpBuf.h"
#include "FinalLog.h"
#include "Message.h"
#include "CommonSettings.h"


namespace {

const char* finallogpath = "final_log/";

}


namespace eyeline {
namespace informer {

struct FinalLog::LogFileFilter
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


FinalLog* FinalLog::instance_ = 0;

FinalLog::FinalLog( const std::vector< std::string >& finpaths ) :
lock_( MTXWHEREAMI ),
finalpaths_(finpaths),
createTime_(0),
period_(60)
{
    assert(!instance_);
    instance_ = this;

    if (finalpaths_.empty()) {
        finalpaths_.push_back(getCS()->getStorePath() + ::finallogpath);
    }
    for ( std::vector< std::string >::iterator it = finalpaths_.begin();
          it != finalpaths_.end(); ++it ) {
        assert(!it->empty());
        if ( (*it)[0] != '/' ) {
            // relative path
            *it = getCS()->getStorePath() + *it;
        }
        if ( (*it)[it->size()-1] != '/' ) {
            *it += '/';
        }
    }

    // check if we have unrolled old files
    std::vector< ulonglong > logfiles;
    std::vector< std::string > dummy;
    LogFileFilter lff(logfiles);
    try {
        makeDirListing( lff, S_IFREG ).list( (getCS()->getStorePath() +
                                              ::finallogpath).c_str(),
                                             dummy );
    } catch ( ErrnoException& e ) {
        return;
    }
    std::sort( logfiles.begin(), logfiles.end() );
    const ulonglong ymdTime = msgTimeToYmd(currentTimeSeconds());
    for ( std::vector< ulonglong >::const_iterator i = logfiles.begin();
          i != logfiles.end(); ++i ) {
        char fn[100];
        sprintf(fn,"%llu.log",*i);
        if ( *i < ymdTime ) {
            // need to roll
            rollFile(fn);
        } else if ( fg_.isOpened() ) {
            throw InfosmeException(EXC_LOGICERROR,"two open files in %s",::finallogpath);
        } else {
            createTime_ = ymdToMsgTime(*i);
            strcpy(filename_,fn);
            fg_.create((getCS()->getStorePath() + ::finallogpath + fn).c_str(),0666);
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
                            int                  smppStatus,
                            int                  nsms )
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
    TmpBuf<char,1024> buf;
    const int bufsize = sprintf(buf.get(),"%02u,%u,%s,0,%llu,%c,%d,%s,%u,",
                                currentTime % 60, dlvId, userId, msg.msgId,
                                cstate,
                                smppStatus, caddr,
                                nsms );
    if (bufsize < 0) {
        throw InfosmeException(EXC_SYSTEM,"cannot printf to final.log: %d",bufsize);
    }
    buf.setPos(bufsize);
    buf.append(msg.msgUserData.c_str(),msg.msgUserData.size());
    buf.append("\n",1);
    smsc::core::synchronization::MutexGuard mg(lock_);
    doCheckRollFile(currentTime,true);
    if ( currentTime < createTime_ ) {
        // fix for delayed write
        ::memcpy(buf.get(),"00",2);
    }
    fg_.write(buf.get(),buf.getPos());
    // fg_.fsync();
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
    smsc::core::synchronization::MutexGuard mg(lock_);
    doCheckRollFile(currentTime,true);
    if (currentTime < createTime_) {
        ::memcpy(buf,"00",2);
    }
    fg_.write(buf,bufsize);
    fg_.fsync();
}


void FinalLog::checkRollFile( msgtime_type currentTime )
{
    smsc::core::synchronization::MutexGuard mg(lock_);
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
        sprintf(filename_,"%04u%02u%02u%02u%02u.log",
                now.tm_year+1900, now.tm_mon+1, now.tm_mday,
                now.tm_hour, now.tm_min );
        fg_.create((getCS()->getStorePath() + ::finallogpath + filename_).c_str(), 0666, true );
        createTime_ = currentTime - (oldmin - now.tm_min)*60 - now.tm_sec;
        fg_.seek(0, SEEK_END);
        if (fg_.getPos() == 0) {
            const char* header = "#1 SEC,DLVID,USERID,RECTYPE,MSGID,STATE,SMPP,SUBSCRIBER,NSMS,USERDATA\n";
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
    memcpy(fnbuf + stemlen,"csv",4);
    const std::string fromfile = getCS()->getStorePath() + ::finallogpath + fn;
    for ( std::vector< std::string >::const_iterator it = finalpaths_.begin() + 1;
          it != finalpaths_.end(); ++it ) {
        FileGuard::copyfile( fromfile.c_str(), (*it + fnbuf).c_str() );
    }
    FileGuard::renameorcopy( fromfile.c_str(), (finalpaths_.front() + fnbuf).c_str() );
}

} // informer
} // smsc
