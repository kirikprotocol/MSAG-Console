#include <cassert>
#include "RollingFileStream.h"
#include "util/crc32.h"
#include "informer/io/FileGuard.h"
#include "informer/io/FileReader.h"
#include "informer/io/TmpBuf.h"

namespace {

const char* FILEFORMAT = "%04u-%02u-%02u-%02u-%02u-%02u%n";
const char* FILEHEADER = "# VERSION 0001\n";

}

namespace scag2 {
namespace pvss {


smsc::logger::Logger* RollingFileStream::log_ = 0;

/// =================================================================

struct RollingFileStreamReader::RFR : public eyeline::informer::FileReader::RecordReader 
{
    RFR( bool& stopping, ProfileLogStreamRecordParser* rp ) :
    stopping_(stopping), rp_(rp), lines(0), crc32(0), nextFile() {}

    virtual bool isStopping() { return stopping_; }
    /// NOTE: terminated by \n
    virtual size_t recordLengthSize() const { return 0; }

    virtual size_t readRecordLength( size_t filePos,
                                     eyeline::informer::FromBuf& fb ) {
        size_t buflen = fb.getLen() - fb.getPos();
        const char* found = 
            static_cast<const char*>(memchr( fb.getBuf(), '\n', buflen));
        if (!found) {
            // request extra bytes
            return buflen + 1;
        }
        return found - fb.getBuf() + 1;
    }

    virtual bool readRecordData( size_t filePos,
                                 eyeline::informer::FromBuf& fb )
    {
        size_t buflen = fb.getLen() - fb.getPos();
        const char* buf = fb.getBuf();
        if ( buf[0] != '#' ) {
            ++lines;
            crc32 = smsc::util::crc32(crc32,buf,buflen);
        } else {
            // processing meta comments, i.e. version, crc, etc.
        }
        if ( rp_ ) {
            if ( buf[buflen-1] != '\n' ) {
                throw eyeline::informer::FileDataException(filePos,"not terminated by \\n");
            }
            rp_->parseRecord(buf,--buflen);
        }
        return true;
    }
public:
    bool&       stopping_;
    ProfileLogStreamRecordParser* rp_;
    uint32_t    lines;
    uint32_t    crc32;
    std::string nextFile;
};


RollingFileStreamReader::RollingFileStreamReader() :
lines_(0),
crc32_(0),
nextFile_()
{
}


void RollingFileStreamReader::read( const char* fullName,
                                    bool&       stopping,
                                    ProfileLogStreamRecordParser* rp )
{
    eyeline::informer::FileGuard fg;
    fg.ropen(fullName);
    eyeline::informer::FileReader fr(fg);
    RFR reader(stopping,rp);
    eyeline::informer::TmpBuf<char,8192> buf;
    fr.readRecords( buf, reader );
    lines_ = reader.lines;
    crc32_ = reader.crc32;
    nextFile_ = reader.nextFile;
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


size_t RollingFileStream::formatPrefix( char* buf, size_t bufsize, const char* catname ) const throw()
{
    timeval tv;
    gettimeofday(&tv,0);
    tm rtm;
    localtime_r(&tv.tv_sec,&rtm);
    const unsigned msec = unsigned(tv.tv_usec / 1000);
    size_t written = size_t(::snprintf(buf,bufsize,"%02u-%02u %02u:%02u:%02u,%03u %10.10s: ",
                                       rtm.tm_mon+1, rtm.tm_mday,
                                       rtm.tm_hour, rtm.tm_min, rtm.tm_sec, msec,
                                       catname ));
    if ( written >= bufsize ) {
        written = bufsize-1; 
    }
    return written;
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
        if ( st.st_size <= strlen(FILEHEADER) ) {
            // file is ok, use it
            needPostfix_ = ( badlogs.size() > 1 );
            filestamp = badlogs.back();
        }
    } else {
        needPostfix_ = false;
    }
    // open the file
    std::string filename(prefix_);
    char buf[128];
    makeFileSuffix(buf,sizeof(buf),filestamp);
    filename.append(buf);
    file_.Append( filename.c_str() );
    if ( filestamp == now ) {
        file_.Write( FILEHEADER, strlen(FILEHEADER) );
    }
    smsc_log_info(log_,"RollFileStream(%s) init unfinished=%u needfix=%d usingLast=%d",
                  prefix_.c_str(), unsigned(badlogs.size()), needPostfix_,
                  filestamp != now );
    startTime_ = filestamp;
    crc32_ = 0;
    lines_ = 0;
}


void RollingFileStream::update( ProfileLogStream& ps )
{
    smsc_log_info(log_,"RollFileStream(%s) update");
    RollingFileStream& rfs = dynamic_cast<RollingFileStream&>(ps);
    interval_ = rfs.interval_;
    if ( prefix_ != rfs.prefix_ ) {
        // need to reopen the file
        prefix_ = rfs.prefix_;
        if ( file_.isOpened() ) { doRollover( time(0), prefix_.c_str() ); }
    }
    finalSuffix_ = rfs.finalSuffix_;
}


void RollingFileStream::postInitFix( bool& isStopping )
{
    if ( !needPostfix_ ) return;

    typedef std::vector< time_t > BadLogs;
    BadLogs badlogs;
    collectUnfinishedLogs( prefix_.c_str(), badlogs );
    if ( badlogs.empty() ) return;
    smsc_log_info(log_,"RollFileStream(%s) postInitFix unfinished=%u",
                  prefix_.c_str(), unsigned(badlogs.size()) );
    
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
        rfsr.read( filename.c_str(), isStopping, 0 );
        lines = rfsr.getLines();
        crc32 = rfsr.getCrc32();
        if ( rfsr.isFinished() ) {
            throw smsc::util::Exception("file '%s' is already finished?",filename.c_str());
        }

        std::string newname(prefix_);
        {
            size_t lastSlash = newname.rfind('/');
            if ( lastSlash != std::string::npos ) {
                newname.erase(0,lastSlash+1);
            }
            char buf[50];
            makeFileSuffix(buf,sizeof(buf),nextLogTime);
            newname.append(buf);
            newname.append(finalSuffix_);
        }
        smsc_log_info(log_,"fixing file '%s', lines=%u crc32=%x next: %s",
                      filename.c_str(), lines, crc32, newname.c_str() );

        smsc::core::buffers::File oldf;
        oldf.Append(filename.c_str());
        finishFile( oldf, crc32, lines, newname.c_str() );
        if ( -1 == rename( filename.c_str(), (filename + finalSuffix_).c_str() ) ) {
            smsc_log_warn(log_,"cannot rename '%s', errno=%d", filename.c_str(), errno);
            throw smsc::util::Exception("rename('%s') exc: errno=%d",filename.c_str(),errno);
        }
    }
}


time_t RollingFileStream::tryToRoll( time_t now )
{
    time_t rollTime = startTime_ + interval_;
    if ( rollTime < now ) {
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
        filename.append( ::FILEFORMAT );
    }
    std::vector< std::string > entries;
    smsc::core::buffers::File::ReadDir( dirname.c_str(), entries,
                                        smsc::core::buffers::File::rdfFilesOnly|
                                        smsc::core::buffers::File::rdfNoDots );

    for ( std::vector<std::string>::const_iterator i = entries.begin();
          i != entries.end(); ++i ) {

        const size_t isize = i->size();
        if ( isize < 5 ) continue;

        // good extension is found
        ::tm ltm;
        memset(&ltm,0,sizeof(ltm));
        int pos = 0;
        sscanf( i->c_str(), filename.c_str(),
                &ltm.tm_year, &ltm.tm_mon, &ltm.tm_mday,
                &ltm.tm_hour, &ltm.tm_min, &ltm.tm_sec, &pos );
        if ( pos == 0 ) {
            // invalid name
            continue;
        }
        if ( size_t(pos) != isize ) {
            // not all chars read
            continue;
        }

        ltm.tm_year -= 1900;
        --ltm.tm_mon;
        ltm.tm_isdst = -1;
        time_t thetime = mktime(&ltm);
        list.push_back(thetime);
    }
    std::sort(list.begin(),list.end());
}


void RollingFileStream::doRollover( time_t now, const char* pathPrefix )
{
    assert(file_.isOpened());
    eyeline::informer::TmpBuf<char,1024> buf;
    makeFileSuffix( buf.get(), buf.getSize(), now);
    std::string newfileName(pathPrefix);
    newfileName.append( buf.get() );

    smsc::core::buffers::File newf;
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
    ::snprintf(buf, sizeof(buf), "# SUMMARY: lines: %u, crc32: %x, next: ",
               lines, crc32 );
    std::string line(buf);
    line.append(newname);
    line.push_back('\n');
    oldf.Write(line.c_str(),line.size());
}

}
}
