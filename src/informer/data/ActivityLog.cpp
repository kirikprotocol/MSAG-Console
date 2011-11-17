#include <ctime>
#include "ActivityLog.h"
#include "CommonSettings.h"
#include "informer/io/TextEscaper.h"
#include "informer/io/HexDump.h"
#include "informer/io/DirListing.h"
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


bool ActivityLog::readStatistics( FileGuard& fg,
                                  TmpBufBase<char>& buf,
                                  DeliveryStats& ods,
                                  bool& isOldVersion )
{
    // FileGuard fg;
    // fg.ropen( filename.c_str() );
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


void ActivityLog::scanZipToEnd( FileGuard& fg,
                                TmpBufBase<char>& buf,
                                const std::string& fn )
{
    fg.ropen(fn.c_str());
    typedef struct stat mystat;
    mystat st;
    const size_t filesize = fg.getStat(st).st_size;

    char headbuf[100];
    size_t wasread = fg.read(headbuf,sizeof(headbuf)-1);
    headbuf[wasread] = '\0';
    char* eol = strchr(headbuf,'\n');
    if (!eol) {
        throw FileReadException( fn.c_str(), 0, 0,
                                 "header '%s' has no EOL",
                                 headbuf );
    }
    *eol = '\0';
    int pos = 0;
    unsigned version;
    sscanf(headbuf,"#%u ZIPPED%n",&version,&pos);
    if ( size_t(pos) != strlen(headbuf) ) {
        throw FileReadException( fn.c_str(), 0, 0,
                                 "wrong header '%s'",
                                 headbuf);
    } else if ( version != 1 ) {
        throw FileReadException( fn.c_str(), 0, 0,
                                 "wrong version %u in '%s'",
                                 version, headbuf);
    }

    size_t curpos = eol - headbuf + 1;
    while ( true ) {
        fg.seek( curpos );
        // reading the chunks header
        wasread = fg.read(headbuf,sizeof(headbuf)-1);
        headbuf[wasread] = '\0';
        eol = strchr(headbuf,'\n');
        if (!eol) {
            throw FileReadException( fn.c_str(), 0, curpos,
                                     "subheader '%s' has no EOL",
                                     headbuf );
        }
        *eol = '\0';
        pos = 0;
        unsigned year, month, mday, hour, minute;
        unsigned long chunksize;
        sscanf(headbuf,"# %04u %02u %02u %02u %02u %lu%n",
               &year, &month, &mday, &hour, &minute, &chunksize, &pos );
        if ( size_t(pos) != strlen(headbuf) ) {
            throw FileReadException( fn.c_str(), 0, curpos,
                                     "wrong subheader '%s'",
                                     headbuf );
        }
        const size_t headsize = eol - headbuf + 1;
        curpos += headsize;
        if ( curpos + chunksize >= filesize ) {
            // the last chunk found
            fg.seek(curpos);
            break;
        }
        curpos += chunksize;
    }
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
        off += int(hd.hexdumpsize(msg.flags.bufsize()));
        buf.reserve(off+1);
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
            dlvInfo_->incMsgStats(region,currentTime,msg.state,1,fromState,nchunks);
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


void ActivityLog::joinLogs( const std::string& dlvdir, bool hourly )
{
    typedef struct stat mystat;

    const std::string actpath = dlvdir + "activity_log/";
    DirListing< NoDotsNameFilter > dl( NoDotsNameFilter(), S_IFDIR );
    std::vector< std::string > dirs, subdirs;
    dl.list( actpath.c_str(), dirs );
    std::sort( dirs.begin(), dirs.end() );
    FileGuard result;
    std::string resultpath;
    smsc::logger::Logger* thelog = smsc::logger::Logger::getInstance("joinlogs");

    for ( std::vector< std::string >::const_iterator i = dirs.begin(),
          ie = dirs.end(); i != ie; ++i ) {
        const std::string daypath = actpath + *i;
        if ( result.isOpened() ) {
            result.close();
            closeJoinedLog( resultpath, thelog );
        }

        unsigned year, month, mday;
        {
            int pos = 0;
            sscanf(i->c_str(),"%04u.%02u.%02u%n",&year,&month,&mday,&pos);
            if ( !pos || size_t(pos) != i->size() ) {
                smsc_log_warn(thelog,"cannot parse day '%s' in '%s'",
                              i->c_str(),actpath.c_str());
                continue;
            }
        }

        subdirs.clear();
        dl.list( daypath.c_str(), subdirs );
        std::sort( subdirs.begin(), subdirs.end() );
        for ( std::vector< std::string >::const_iterator j = subdirs.begin(),
              je = subdirs.end(); j != je; ++j ) {

            const std::string hourpath = daypath + "/" + *j;
            if ( hourly && result.isOpened() ) {
                result.close();
                closeJoinedLog(resultpath,thelog);
            }

            try {

                unsigned hour;
                {
                    int pos = 0;
                    sscanf(j->c_str(),"%02u%n",&hour,&pos);
                    if ( !pos || size_t(pos) != j->size() ) {
                        throw InfosmeException(EXC_BADFILE, 
                                               "cannot parse hour '%s' in '%s'",
                                               j->c_str(),daypath.c_str());
                    }
                }

                std::vector< std::string > logfiles;
                makeDirListing( NoDotsNameFilter(), S_IFREG ).list( hourpath.c_str(), logfiles );
                std::sort( logfiles.begin(), logfiles.end() );

                for ( std::vector< std::string >::const_iterator k = logfiles.begin(),
                      ke = logfiles.end(); k != ke; ++k ) {

                    unsigned minute;
                    {
                        int pos = 0;
                        sscanf(k->c_str(),"%02u.log%n",&minute,&pos);
                        if ( !pos || size_t(pos) != k->size() ) {
                            throw InfosmeException( EXC_BADFILE, "cannot parse '%s' in '%s'",
                                                    k->c_str(), hourpath.c_str() );
                        }
                    }

                    const std::string fn = hourpath + "/" + *k;

                    FileGuard from;
                    from.ropen(fn.c_str());
                    // read/check version -- not needed

                    // check EOL @ EOF
                    mystat st;
                    from.getStat(st);
                    from.seek( st.st_size-1 );
                    char tbuf[1];
                    if ( 1 != from.read(tbuf,1) ) {
                        throw FileReadException( fn.c_str(), 0, st.st_size-1, 
                                                 "cannot read EOL@EOF" );
                    } else if ( tbuf[0] != '\n' ) {
                        throw FileReadException( fn.c_str(), 0, st.st_size-1,
                                                 "%x at EOF, must be EOL",
                                                 static_cast<unsigned char>(tbuf[1]) );
                    }
                    from.seek(0);

                    // create result file if not opened
                    if ( !result.isOpened() ) {
                        resultpath = 
                            ( hourly ? hourpath : daypath );
                        result.create((resultpath + ".tmp").c_str(),
                                      0666, false, true );
                        const char* ziphead = "#1 ZIPPED\n";
                        result.write(ziphead,strlen(ziphead));
                    }

                    // appending the logfile
                    char headbuf[100];
                    sprintf(headbuf,"# %04u %02u %02u %02u %02u %lu\n",
                            year, month, mday, hour, minute,
                            static_cast<unsigned long>(st.st_size) );
                    result.write(headbuf,strlen(headbuf));

                    char wbuf[8192];
                    while ( true ) {
                        const size_t wasread = from.read(wbuf,sizeof(wbuf));
                        if (wasread == 0) { break; }
                        result.write(wbuf,wasread);
                    }

                } // for logfiles
            } catch ( std::exception& e ) {
                // failed to process a file in the hour
                smsc_log_warn(thelog,"exc: %s",e.what());
                if (result.isOpened()) {
                    result.close();
                    try {
                        FileGuard::unlink( (resultpath + ".tmp").c_str() );
                    } catch (...) {}
                }
                if (!hourly) break; // to the next day
            }
        } // loop over hours
    } // loop over days

    if ( result.isOpened() ) {
        result.close();
        closeJoinedLog(resultpath,thelog);
    }
}


void ActivityLog::closeJoinedLog( const std::string& resultpath,
                                  smsc::logger::Logger* thelog )
{
    if ( 0 == ::rename( (resultpath + ".tmp").c_str(),
                        (resultpath + ".log").c_str() ) ) {
        // success
        // destroy all subdirectories
        /*
         * FIXME: temporary do not delete dirs
        try {
            FileGuard::rmdirs( (resultpath + "/").c_str() );
        } catch ( std::exception& e ) {
            smsc_log_warn(thelog,"could not delete '%s'",resultpath.c_str());
        }
         */
    } else {
        smsc_log_warn(thelog,"could not rename '%s.tmp'",resultpath.c_str());
    }
}

}
}
