#ifndef PVSS_ROLLINGFILESTREAM_H
#define PVSS_ROLLINGFILESTREAM_H

#include "ProfileLog.h"
#include "core/buffers/File.hpp"

namespace scag2 {
namespace pvss {

class RollingFileStream : public ProfileLogStream
{
    friend class ProfileLogRoller;
public:
    RollingFileStream( const char* name,
                       const char* prefix,
                       const char* finalSuffix,
                       unsigned    interval );

protected:
    virtual size_t formatPrefix( char* buf, size_t bufsize ) const throw();
    virtual void write( const char* buf, size_t bufsize );

    virtual void init();
    virtual void update( ProfileLogStream& ps );

    virtual void postInitFix( bool& isStopping );
    virtual time_t tryToRoll( time_t now );

    void doRollover( time_t now, const char* pathPrefix );

    static void collectUnfinishedLogs( const char* prefix, std::vector< time_t >& list );
    static void makeFileSuffix( char* buf, size_t bufsize, time_t now ) throw();
    static void finishFile( smsc::core::buffers::File& oldf,
                            uint32_t crc32, uint32_t lines,
                            const char* newname );

private:
    std::string               prefix_;
    std::string               finalSuffix_;
    unsigned                  interval_;

    time_t                    startTime_;
    smsc::core::buffers::File file_;
    uint32_t                  crc32_;
    uint32_t                  lines_;
};

}
}

#endif
