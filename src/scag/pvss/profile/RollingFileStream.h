#ifndef PVSS_ROLLINGFILESTREAM_H
#define PVSS_ROLLINGFILESTREAM_H

#include "ProfileLog.h"
#include "core/buffers/File.hpp"
#include "logger/Logger.h"

namespace scag2 {
namespace pvss {

class RollingFileStreamReader
{
    struct RFR;
public:
    RollingFileStreamReader();
    
    void read( const char*                   fullName,
               volatile bool*                isStopping,
               ProfileLogStreamRecordParser* rp );
    
    std::string readNextFile( const char* fullName );

    uint32_t getLines() const { return lines_; }
    uint32_t getCrc32() const { return crc32_; }
    bool isFinished() const { return !nextFile_.empty(); }
    const std::string& getNextFile() const { return nextFile_; }

private:
    uint32_t    lines_;     // counted
    uint32_t    crc32_;     // counted
    std::string nextFile_;  // read from the stream
};


class RollingFileStream : public ProfileLogStream
{
    friend class ProfileLogRoller;
public:
    RollingFileStream( const char* name,
                       const char* prefix,
                       const char* finalSuffix,
                       unsigned    interval );

    /// return 0 on failure
    static time_t extractTime( const char* filename,
                               const char* prefix,
                               const char* suffix );
    /// scan the prefix of the record, filling ltm and catname
    /// @return the number of chars consumed by prefix.
    /// NOTE: may throw exception on invalid prefix.
    static size_t scanPrefix( const char* buf,
                              size_t bufsize,
                              ::tm& ltm, std::string& catname );

protected:
    virtual size_t formatPrefix( char* buf, size_t bufsize, const char* catname ) const throw();
    virtual void write( const char* buf, size_t bufsize );

    virtual void init();
    virtual void update( ProfileLogStream& ps );

    virtual void postInitFix( volatile bool& isStopping );
    virtual time_t tryToRoll( time_t now );

    void doRollover( time_t now, const char* pathPrefix );

    void collectUnfinishedLogs( const char* prefix, std::vector< time_t >& list ) const;
    void makeFileSuffix( char* buf, size_t bufsize, time_t now ) const throw();
    void finishFile( smsc::core::buffers::File& oldf,
                     uint32_t crc32, uint32_t lines,
                     const char* newname ) const;

    static void readTheFile( const char* filename,
                             uint32_t& lines,
                             uint32_t& crc32 );

private:
    static smsc::logger::Logger* log_;
    std::string                  prefix_;
    std::string                  finalSuffix_;
    unsigned                     interval_;
                                 
    time_t                       startTime_;
    smsc::core::buffers::File    file_;
    uint32_t                     crc32_;
    uint32_t                     lines_;
};

}
}

#endif
