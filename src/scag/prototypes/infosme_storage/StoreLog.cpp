#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdio>
#include "StoreLog.h"
#include "scag/util/io/EndianConverter.h"
#include "util/Exception.hpp"

using scag2::util::io::EndianConverter;

namespace scag2 {
namespace prototypes {
namespace infosme {

StoreLog::StoreLog( const char* fpath ) :
log_(smsc::logger::Logger::getInstance("storelog")),
fd_(-1), version_(1)
{
    if ( !fpath ) {
        throw smsc::util::Exception("StoreLog: file path is null");
    }
    fd_ = open(fpath, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    if ( fd_ == -1 ) {
        throw smsc::util::Exception("StoreLog: file %s cannot be opened: %d",fpath,errno);
    }
}


StoreLog::~StoreLog()
{
    if (fd_ != -1) close(fd_);
    fd_ = -1;
}


void StoreLog::writeMessage( dlvid_type    dlvId,
                             regionid_type regionId,
                             Message&      msg )
{
    /*
    if (fd_ == -1) {
        throw smsc::util::Exception("StoreLog: not opened");
    }
    unsigned char buf[200];
    unsigned char* p = buf;
    p += 2; // reserve place for the length
    EndianConverter::set16( p, uint16_t(version_) );
    p += 2;
    EndianConverter::set32( p, uint32_t(taskId));
    p += 4;
    EndianConverter::set32( p, uint32_t(regionId));
    p += 4;
    // message
    p = msg.toBuf( uint16_t(version_), p );
    const uint16_t len = (p - buf) + 2;
    EndianConverter::set16(buf, len);
    EndianConverter::set16(p, len);
    ssize_t res = write(fd_, buf, size_t(len));
    if ( res == -1 ) {
        throw smsc::util::Exception("StoreLog: cannot write message TRM=(%u/%u/%u): %d",
                                    taskId, regionId, msg.msgId, errno );
    } else if ( res != ssize_t(len) ) {
        throw smsc::util::Exception("StoreLog: different number of bytes written %d, must be %d",
                                    int(res), int(len));
    }
     */

    char buf[200];
    char* p = buf;
    p += sprintf(buf,"%u,%u,%u,",version_,dlvId,regionId);
    p = msg.printToBuf(version_,p);
    // *p = '\n';
    // *++p = '\0';
    // const int fd = fileno(stdout);
    // write(fd,buf,size_t(p-buf));
    // fsync(fd);
    smsc_log_debug(log_,"%s",buf);
}

}
}
}
