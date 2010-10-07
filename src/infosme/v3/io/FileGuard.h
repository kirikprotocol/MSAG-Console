#ifndef _INFORMER_FILEGUARD_H
#define _INFORMER_FILEGUARD_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "InfosmeException.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

class FileGuard
{
public:
    FileGuard() : fd_(-1) {
        if (!log_) {
            log_ = smsc::logger::Logger::getInstance("fileguard");
        }
    }

    ~FileGuard() {
        close();
    }

    inline bool isOpened() const { return fd_ != -1; }

    void ropen( const char* fn );

    void create( const char* fn, bool mkdirs = false, bool truncate = false );

    void seek( uint32_t pos ) {
        if (fd_!=-1) lseek(fd_,off_t(pos),SEEK_SET);
    }

    /// write buffer
    /// @a atomic - true: generate exception if write was not atomic
    void write( const void* buf, unsigned buflen, bool atomic = false );

    /// read buffer
    /// @return number of bytes read
    unsigned read( void* buf, unsigned buflen );

    void fsync() {
        if (fd_!=-1) ::fsync(fd_);
    }

    void close() {
        if (fd_!=-1) ::close(fd_);
        fd_ = -1;
    }

    static void makedirs( const std::string& dir );

private:
    static smsc::logger::Logger* log_;
    int fd_;
};

} // informer
} // smsc

#endif
