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
        try {
            close();
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"fn='%s' exc at close: %s",fn_.c_str(),e.what());
        } catch (...) {
            smsc_log_error(log_,"fn='%s' exc at closing the file",fn_.c_str());
        }
    }

    inline bool isOpened() const {
        return fd_ != -1;
    }

    inline const char* getFileName() const {
        return fn_.c_str();
    }

    void swap( FileGuard& fg ) {
        std::swap(fd_,fg.fd_);
        std::swap(pos_,fg.pos_);
        fn_.swap(fg.fn_);
    }

    void ropen( const char* fn );

    void create( const char* fn,
                 mode_t      mode = 0666,
                 bool        mkdirs = false,
                 bool        truncate = false );

    size_t seek( off_t pos, int whence = SEEK_SET );

    inline size_t getPos() const { return pos_; }

    /// get stat
    const struct stat& getStat( struct stat& st ) const;

    /// write buffer
    /// @a atomic - true: generate exception if write was not atomic
    void write( const void* buf, size_t buflen, bool atomic = false );

    /// read buffer
    /// @return number of bytes read which may be less than buflen in case of EOF
    size_t read( void* buf, size_t buflen );

    void fsync() {
        if (fd_!=-1) {
            if ( 0 != ::fsync(fd_) ) {
                throw FileWriteException(fn_.c_str(),errno,"fsync");
            }
        }
    }

    /// NOTE: pos is preserved
    void truncate( size_t pos );

    void close( bool dosync = true );

    /// unlink a file
    static void unlink( const char* fname );

    static void makedirs( const std::string& dir );

    /// unlink directory contents recursively (optionally).
    /// @param path must ends with '/'
    static void rmdirs( const char* path, bool rmself = true );

    /// copy directory recursively, both from and to must ends with '/'
    static void copydir( const char* from,
                         const std::string& to,
                         unsigned maxdepth );

    /// copy file.
    /// \param tempext is the extension of temporary file ${to} + ${tempext}.
    /// then this temporary file is renamed into final one.
    /// if tempext is empty, then temporary file is not used.
    static void copyfile( const char* from,
                          const char* to,
                          const char* tempext = ".tmpcopy" );
    
    /// rename the file, or (if failed) try to copy.
    static void renameorcopy( const char* from,
                              const char* to,
                              const char* tempext = ".tmpcopy" );

private:
    inline static void getlog() {
        if (!log_) {
            log_ = smsc::logger::Logger::getInstance("fileguard");
        }
    }
    static smsc::logger::Logger* log_;

private:
    std::string fn_;
    int         fd_;
    size_t      pos_;
};

} // informer
} // smsc

#endif
