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
    FileGuard() : fd_(-1), pos_(0) {
        getlog();
    }

    ~FileGuard() {
        close();
    }

    inline bool isOpened() const {
        return fd_ != -1;
    }

    void swap( FileGuard& fg ) {
        std::swap(fd_,fg.fd_);
        std::swap(pos_,fg.pos_);
    }

    void ropen( const char* fn );

    void create( const char* fn, bool mkdirs = false, bool truncate = false );

    size_t seek( size_t pos, int whence = SEEK_SET );

    inline size_t getPos() const { return pos_; }

    /// write buffer
    /// @a atomic - true: generate exception if write was not atomic
    void write( const void* buf, size_t buflen, bool atomic = false );

    /// read buffer
    /// @return number of bytes read which may be less than buflen in case of EOF
    size_t read( void* buf, size_t buflen );

    void fsync() {
        if (fd_!=-1) ::fsync(fd_);
    }

    /// NOTE: pos is preserved
    void truncate( size_t pos );

    void close() {
        if (fd_!=-1) ::close(fd_);
        fd_ = -1;
        pos_ = 0;
    }

    static void makedirs( const std::string& dir );

private:
    inline static void getlog() {
        if (!log_) {
            log_ = smsc::logger::Logger::getInstance("fileguard");
        }
    }
    static smsc::logger::Logger* log_;

private:
    int    fd_;
    size_t pos_;
};

} // informer
} // smsc

#endif
