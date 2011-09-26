#include <cassert>
#include "RollingFileStream.h"
#include "util/crc32.h"
#include "informer/io/FileGuard.h"
#include "informer/io/FileReader.h"
#include "informer/io/TmpBuf.h"

using eyeline::informer::InfosmeException;
using eyeline::informer::FileReadException;
using eyeline::informer::FileGuard;
using eyeline::informer::FileReader;
using eyeline::informer::TmpBuf;
using eyeline::informer::EXC_IOERROR;

namespace {

const char* FILEFORMAT = "%04u-%02u-%02u-%02u-%02u-%02u%n";
const char* FILEHEADER = "# VERSION 0001\n";
const char* FILETRAILER = "# SUMMARY: lines: %u, crc32: %08x, next: %n";
const char* PREFIXFORMAT = "%02u-%02u %02u:%02u:%02u,%03u %10.10s: %n";
const char* PREFIXFORMATIN = "%02u-%02u %02u:%02u:%02u,%03u %10[^:]: %n";

}

namespace scag2 {
namespace pvss {


smsc::logger::Logger* RollingFileStream::log_ = 0;

/// =================================================================

struct RollingFileStreamReader::RFR : public eyeline::informer::FileReader::RecordReader
{
    RFR( volatile bool* stopping, ProfileLogStreamRecordParser* rp ) :
    stopping_(stopping), rp_(rp), lines(0), crc32(0), nextFile() {}

    virtual bool isStopping() { return stopping_ ? *stopping_ : false; }
    /// NOTE: terminated by \n
    virtual size_t recordLengthSize() const { return 0; }

    virtual size_t readRecordLength( size_t filePos,
                                     char* buf, size_t buflen ) 
    {
        const char* found = 
            static_cast<const char*>(memchr(buf,'\n',buflen));
        if (!found) {
            // request extra bytes
            return buflen + 1;
        }
        return found - buf + 1;
    }

    virtual bool readRecordData( size_t filePos, char* buf, size_t buflen );
public:
    volatile bool*       stopping_;
    ProfileLogStreamRecordParser* rp_;
    uint32_t    lines;
    uint32_t    crc32;
    std::string nextFile;
};


bool RollingFileStreamReader::RFR::readRecordData( size_t filePos,
                                                   char* buf,
                                                   size_t buflen )
{
    char cr = '\0';
    const bool res = (buf[0] != '#');
    if ( res ) {
        ++lines;
        crc32 = smsc::util::crc32(crc32,buf,buflen);
        std::swap(buf[buflen-1],cr);
    } else if ( filePos == 0 ) {
        // version ?
        if ( 0 != strncmp(buf,FILEHEADER,strlen(FILEHEADER)) ) {
            throw FileReadException("",0,filePos,"not started with VERSION");
        }
        std::swap(buf[buflen-1],cr);
    } else {
        std::swap(buf[buflen-1],cr);
        int pos = 0;
        // trying to read trailer
        unsigned linesRead, crcRead;
        sscanf(buf,FILETRAILER,&linesRead,&crcRead,&pos);
        if ( pos != 0 ) {
            // it is a trailer
            if ( ! nextFile.empty() ) {
                throw FileReadException("",0,filePos,"duplicate trailer lines, search with grep '^#'");
            }
            if ( rp_ ) {
                // if rp is not set, then we only need to extract nextfile
                if ( lines != linesRead ) {
                    throw FileReadException("",0,filePos,"not matched: linesInTail=%u lines=%u",
                                            linesRead, lines );
                } else if ( crcRead != crc32 ) {
                    throw FileReadException("",0,filePos,"not matched: crcInTail=%08x crc32=%08x",
                                            crcRead, crc32 );
                }
            }
            // get the next file name
            nextFile = buf+pos;
        }
    }
    if (cr!='\n') {
        throw FileReadException("",0,filePos,"not terminated by \\n");
    }
    if ( rp_ ) {
        rp_->parseRecord(buf,--buflen);
    }
    return res;
}


RollingFileStreamReader::RollingFileStreamReader() :
lines_(0),
crc32_(0),
nextFile_()
{
}


void RollingFileStreamReader::read( const char*    fullName,
                                    volatile bool* stopping,
                                    ProfileLogStreamRecordParser* rp )
{
    FileGuard fg;
    fg.ropen(fullName);
    FileReader fr(fg);
    RFR reader(stopping,rp);
    TmpBuf<char,8192> buf;
    fr.readRecords( buf, reader );
    lines_ = reader.lines;
    crc32_ = reader.crc32;
    nextFile_ = reader.nextFile;
}


std::string RollingFileStreamReader::readNextFile( const char* fullName )
{
    FileGuard fg;
    fg.ropen(fullName);
    struct stat st;
    TmpBuf<char,1024> tailbuf;
    size_t pos = 0;
    if ( fg.getStat(st).st_size > tailbuf.getSize() ) {
        pos = st.st_size - tailbuf.getSize();
    }
    fg.seek(pos);
    size_t wasread = fg.read(tailbuf.get(),tailbuf.getSize());
    RFR rfr(0,0);
    char* ptr = tailbuf.get();
    if ( pos > 0 ) {
        ptr = static_cast<char*>(memchr(ptr,'\n',wasread));
        if ( !ptr ) {
            throw InfosmeException(EXC_IOERROR,"linefeed is not found");
        }
        const size_t offset = ptr - tailbuf.get();
        wasread -= offset;
        pos += offset;
    }
    while ( wasread > 0 ) {
        size_t reclen = rfr.readRecordLength(pos,ptr,wasread);
        if ( reclen > wasread ) {
            throw FileReadException(fullName,0,pos,"file is garbled in readNextFile");
        }
        try {
            rfr.readRecordData(pos,ptr,reclen);
        } catch ( FileReadException& e ) {
            throw FileReadException(e,fullName);
        }
        wasread -= reclen;
        pos += reclen;
        ptr += reclen;
    }
    return rfr.nextFile;
}


/// =================================================================

RollingFileStream::RollingFileStream( const char* name,
                                      const char* prefix,
                                      const char* finalSuffix,
                                      unsigned  interval ) :
ProfileLogStream(name),
prefix_(prefix), finalSuffix_(finalSuffix),
interval_(interval)
{
    if (!log_) { log_ = smsc::logger::Logger::getInstance("rollfile"); }
}



time_t RollingFileStream::extractTime( const char* filename,
                                       const char* prefix,
                                       const char* suffix )
{
    if ( !filename || !prefix || !suffix ) return 0;
    const size_t fl = strlen(filename), pl = strlen(prefix), sl = strlen(suffix);
    if ( fl <= pl+sl ) return 0;
    if ( memcmp(filename,prefix,pl) ) return 0;
    if ( memcmp(filename+fl-sl,suffix,sl) ) return 0;
    int pos = 0;
    ::tm ltm;
    sscanf( filename+pl, FILEFORMAT,
            &ltm.tm_year, &ltm.tm_mon, &ltm.tm_mday,
            &ltm.tm_hour, &ltm.tm_min, &ltm.tm_sec, &pos );
    if ( pos == 0 ) return 0;
    if ( pl+pos+sl != fl ) return 0;
    ltm.tm_year -= 1900;
    --ltm.tm_mon;
    ltm.tm_isdst = -1;
    return mktime(&ltm);
}



size_t RollingFileStream::formatPrefix( char* buf, size_t bufsize, const char* catname ) const throw()
{
    timeval tv;
    gettimeofday(&tv,0);
    tm rtm;
    localtime_r(&tv.tv_sec,&rtm);
    const unsigned msec = unsigned(tv.tv_usec / 1000);
    int pos;
    size_t written = size_t(::snprintf(buf,bufsize,
                                       ::PREFIXFORMAT,
                                       rtm.tm_mon+1, rtm.tm_mday,
                                       rtm.tm_hour, rtm.tm_min,
                                       rtm.tm_sec, msec,
                                       catname, &pos ));
    if ( written >= bufsize ) {
        written = bufsize-1; 
    }
    return written;
}


size_t RollingFileStream::scanPrefix( const char* buf,
                                      size_t bufsize,
                                      tm& ltm, std::string& cat )
{
    memset(&ltm,0,sizeof(tm));
    char catname[20];
    unsigned msec;
    int pos = 0;
    sscanf( buf, ::PREFIXFORMATIN,
            &ltm.tm_mon, &ltm.tm_mday,
            &ltm.tm_hour, &ltm.tm_min,
            &ltm.tm_sec, &msec,
            catname, &pos );
    if ( pos <= 0 || size_t(pos) > bufsize ) {
        throw InfosmeException( EXC_IOERROR,
                                "invalid prefix in line '%.*s'",
                                int(bufsize), buf );
    }
    cat = catname;
    return size_t(pos);
}


void RollingFileStream::write( const char* buf, size_t bufsize )
{
    // count crc32 and lines
    smsc::core::synchronization::MutexGuard mg(lock_);
    if ( !file_.isOpened() ) {
        fprintf(stderr,"logic problem in ProfileLog: file is not opened");
        std::terminate();
        return;
    }
    file_.Write(buf,bufsize);
    crc32_ = smsc::util::crc32(crc32_,buf,bufsize);
    ++lines_;
}


void RollingFileStream::init()
{
    smsc::core::synchronization::MutexGuard mg(lock_);
    // open all necessary files, set needpostfix flag if needed
    std::vector< time_t > badlogs;
    collectUnfinishedLogs( prefix_.c_str(), badlogs );
    time_t now = time(0);
    time_t filestamp = now;
    if ( !badlogs.empty()) {
        // check the last file
        needPostfix_ = true;
        char buf[128];
        makeFileSuffix(buf,sizeof(buf),badlogs.back());
        std::string filename(prefix_);
        filename.append(buf);
        struct stat st;
        if ( -1 == stat(filename.c_str(),&st) ) {
            throw smsc::util::Exception("logic error: file '%s' cannot be stated errno=%d",filename.c_str(),errno);
        } else if ( ! S_ISREG(st.st_mode) ) {
            throw smsc::util::Exception("file '%s' is not a regular file");
        }
        smsc_log_info(log_,"RollFileStream(%s) last unfinished file: %s",
                      getName(), filename.c_str());
        if ( st.st_size <= strlen(FILEHEADER) ) {
            // file is ok, use it
            needPostfix_ = ( badlogs.size() > 1 );
            filestamp = badlogs.back();
        } else if ( badlogs.back() >= now ) {
            // we have to wait until we get the next file
            filestamp = badlogs.back() + 1;
            if ( filestamp < now + 5 ) {
                // we may simply wait
                do {
                    now = time(0);
                    if ( filestamp <= now ) { break; }
                    smsc_log_warn(log_,"filestamp %ld is in the future, waiting %d seconds",
                                  long(filestamp), int(filestamp-now));
                    timespec ts = {filestamp-now,0};
                    ::nanosleep(&ts,0);
                } while ( true );
            } else {
                smsc_log_warn(log_,"filestamp %ld is in future %d seconds, but we will use it anyway",
                              long(filestamp),int(filestamp-now));
            }
        }
    } else {
        needPostfix_ = false;
    }
    // open the file
    std::string filename(prefix_);
    char buf[128];
    makeFileSuffix(buf,sizeof(buf),filestamp);
    filename.append(buf);
    file_.SetUnbuffered();
    file_.Append( filename.c_str() );
    smsc_log_debug(log_,"RollFileStream(%s) opening %s",getName(),filename.c_str());
    if ( file_.Pos() == 0 ) {
        file_.Write( FILEHEADER, strlen(FILEHEADER) );
    }
    smsc_log_info(log_,"RollFileStream(%s) init unfinished=%u needfix=%d usingLast=%d",
                  getName(), unsigned(badlogs.size()), needPostfix_,
                  filestamp != now );
    startTime_ = filestamp;
    crc32_ = 0;
    lines_ = 0;
}


void RollingFileStream::update( ProfileLogStream& ps )
{
    smsc_log_info(log_,"RollFileStream(%s) update",getName());
    RollingFileStream& rfs = dynamic_cast<RollingFileStream&>(ps);
    interval_ = rfs.interval_;
    if ( prefix_ != rfs.prefix_ ) {
        // need to reopen the file
        prefix_ = rfs.prefix_;
        if ( file_.isOpened() ) { doRollover( time(0), prefix_.c_str() ); }
    }
    finalSuffix_ = rfs.finalSuffix_;
}


void RollingFileStream::postInitFix( volatile bool& isStopping )
{
    if ( !needPostfix_ ) return;

    typedef std::vector< time_t > BadLogs;
    BadLogs badlogs;
    collectUnfinishedLogs( prefix_.c_str(), badlogs );
    if ( badlogs.empty() ) return;
    smsc_log_info(log_,"RollFileStream(%s) postInitFix unfinished=%u",
                  getName(), unsigned(badlogs.size()) );
    
    for ( BadLogs::iterator j = badlogs.begin(); ; ) {
        time_t currentLogTime = *j;
        ++j;
        if ( j == badlogs.end() ) { break; }
        time_t nextLogTime = *j;
        if ( isStopping ) return;

        // fixing the log file
        uint32_t crc32 = 0;
        uint32_t lines = 0;
        std::string filename(prefix_);
        {
            char buf[50];
            makeFileSuffix(buf,sizeof(buf),currentLogTime);
            filename.append(buf);
        }
        smsc_log_debug(log_,"scanning the file '%s'",filename.c_str());
        RollingFileStreamReader rfsr;
        try {
            rfsr.read( filename.c_str(), &isStopping, 0 );
            lines = rfsr.getLines();
            crc32 = rfsr.getCrc32();
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"file '%s' read exc: %s",filename.c_str(),e.what());
            continue;
        }
        if ( rfsr.isFinished() ) {
            throw smsc::util::Exception("file '%s' is already finished?",filename.c_str());
        }

        std::string newname(prefix_);
        {
            size_t lastSlash = newname.rfind('/');
            if ( lastSlash != std::string::npos ) {
                newname.erase(0,lastSlash+1);
            }
            char nextTimeBuf[50];
            makeFileSuffix(nextTimeBuf,sizeof(nextTimeBuf),nextLogTime);
            newname.append(nextTimeBuf);
            newname.append(finalSuffix_);
        }
        smsc_log_info(log_,"fixing file '%s', lines=%u crc32=%x next: %s",
                      filename.c_str(), lines, crc32, newname.c_str() );

        smsc::core::buffers::File oldf;
        oldf.Append(filename.c_str());
        finishFile( oldf, crc32, lines, newname.c_str() );
        if ( -1 == rename( filename.c_str(), (filename + finalSuffix_).c_str() ) ) {
            smsc_log_error(log_,"cannot rename '%s', errno=%d", filename.c_str(), errno);
            throw smsc::util::Exception("rename('%s') exc: errno=%d",filename.c_str(),errno);
        }
    }
}


time_t RollingFileStream::tryToRoll( time_t now )
{
    time_t rollTime = startTime_ + interval_;
    if ( rollTime <= now ) {
        doRollover( now, prefix_.c_str() );
        rollTime = startTime_ + interval_;
    }
    return rollTime;
}


void RollingFileStream::collectUnfinishedLogs( const char* prefix,
                                               std::vector< time_t >& list ) const
{
    std::string dirname(prefix);
    std::string filename;
    {
        const size_t lastSlash = dirname.rfind('/');
        if (lastSlash != std::string::npos) {
            std::string(dirname,lastSlash+1).swap(filename);
            dirname.erase(lastSlash);
        } else {
            filename = dirname;
            dirname = ".";
        }
    }
    std::vector< std::string > entries;
    smsc::core::buffers::File::ReadDir( dirname.c_str(), entries,
                                        smsc::core::buffers::File::rdfFilesOnly|
                                        smsc::core::buffers::File::rdfNoDots );

    for ( std::vector<std::string>::const_iterator i = entries.begin();
          i != entries.end(); ++i ) {

        const time_t res = extractTime( i->c_str(), filename.c_str(), "");
        if ( res ) { list.push_back(res); }

    }
    std::sort(list.begin(),list.end());
}


void RollingFileStream::doRollover( time_t now, const char* pathPrefix )
{
    assert(file_.isOpened());
    TmpBuf<char,1024> buf;
    makeFileSuffix( buf.get(), buf.getSize(), now);
    std::string newfileName(pathPrefix);
    newfileName.append( buf.get() );

    smsc::core::buffers::File newf;
    newf.SetUnbuffered();
    newf.Append( newfileName.c_str() );
    newf.Write( FILEHEADER, strlen(FILEHEADER) );

    // strip directory
    std::string nextfile;
    size_t slash = newfileName.rfind('/');
    if ( slash == std::string::npos ) {
        nextfile = newfileName;
    } else {
        std::string(newfileName,slash+1).swap(nextfile);
    }
    nextfile.append(finalSuffix_);

    std::string oldname(pathPrefix);
    {
        char suff[128];
        makeFileSuffix(suff,sizeof(suff),startTime_);
        oldname.append(suff);
    }
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        finishFile( file_, crc32_, lines_, nextfile.c_str() );
        file_.Swap( newf );
        startTime_ = now;
        crc32_ = 0;
        lines_ = 0;
    }
    if ( -1 == rename(oldname.c_str(),(oldname+finalSuffix_).c_str()) ) {
        smsc_log_warn(log_,"rename('%s') exc: errno=%d",oldname.c_str(),errno);
    }
}


void RollingFileStream::makeFileSuffix( char* buf, size_t bufsize, time_t now ) const throw()
{
    ::tm ltm;
    ::localtime_r(&now,&ltm);
    int pos = 0;
    ::snprintf( buf, bufsize, ::FILEFORMAT,
                ltm.tm_year+1900, ltm.tm_mon+1, ltm.tm_mday,
                ltm.tm_hour, ltm.tm_min, ltm.tm_sec, &pos );
}


void RollingFileStream::finishFile( smsc::core::buffers::File& oldf,
                                    uint32_t crc32, uint32_t lines,
                                    const char* newname ) const
{
    smsc_log_debug(log_,"finishing current file: lines=%u crc32=%x next=%s",
                   lines, crc32, newname );
    char buf[80];
    int pos;
    ::snprintf(buf, sizeof(buf), ::FILETRAILER, lines, crc32, &pos );
    std::string line(buf);
    line.append(newname);
    line.push_back('\n');
    oldf.Write(line.c_str(),line.size());
}

}
}
