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
    /*
    FileGuard(const char* fn, int flags) : fd_(-1)
    {
        reset(fn,flags);
    }
     */

    ~FileGuard() {
        close();
    }

    inline bool isOpened() const { return fd_ != -1; }

    void ropen( const char* fn ) {
        // open file readonly
        close();
        if (fn && fn[0]) {
            fd_ = open(fn,O_RDONLY);
            if (fd_==-1) {
                const size_t buflen = 100;
                char buf[buflen];
                strerror_r(errno,buf,buflen);
                throw InfosmeException("cannot open '%s' for reading: %d, %s", fn, errno, buf);
            }
        }
    }

    void create( const char* fn, bool mkdirs = false, bool truncate = false ) {
        // create file, open for writing
        close();
        if (fn && fn[0]) {
            int flags = O_WRONLY|O_CREAT;
            if (truncate) flags |= O_TRUNC;
            if (mkdirs) {
                std::string dir;
                const char* p = strrchr(fn,'/');
                if (p) {
                    dir.assign(fn,p-fn);
                } else {
                    dir = ".";
                }
                makedirs( dir );
            }
            fd_ = open(fn,flags,0777);
            if (fd_==-1) {
                const size_t buflen = 100;
                char buf[buflen];
                strerror_r(errno,buf,buflen);
                throw InfosmeException("cannot open '%s' for writing: %d", fn, errno);
            }
        }
    }

    void seek( uint32_t pos ) {
        if (fd_!=-1) lseek(fd_,off_t(pos),SEEK_SET);
    }

    void write( const void* buf, unsigned buflen ) {
        if (fd_==-1) {
            throw InfosmeException("write failed: file is not opened");
        }
        while (buflen>0) {
            ssize_t written = ::write(fd_,buf,buflen);
            if (written == -1) {
                const size_t ebuflen = 100;
                char ebuf[ebuflen];
                strerror_r(errno,ebuf,ebuflen);
                throw InfosmeException("write failed: %d, %s",fd_,errno,ebuf);
            }
            buflen -= written;
        }
    }

    int read( void* buf, unsigned buflen ) {
        if (fd_==-1) {
            throw InfosmeException("read failed: file is not opened");
        }
        int wasread = 0;
        while (buflen>0) {
            ssize_t readlen = ::read(fd_,buf,buflen);
            if (readlen==0) break; // EOF
            else if (readlen==-1) {
                const size_t ebuflen = 100;
                char ebuf[ebuflen];
                strerror_r(errno,ebuf,ebuflen);
                throw InfosmeException("read failed: %d, %s", fd_, errno, ebuf);
            }
            wasread += readlen;
            buflen -= readlen;
            buf = reinterpret_cast<char*>(buf) + readlen;
        }
        return wasread;
    }

    void fsync() {
        if (fd_!=-1) ::fsync(fd_);
    }

    void close() {
        if (fd_!=-1) ::close(fd_);
        fd_ = -1;
    }

    static void makedirs( const std::string& dir )
    {
        std::string work;
        work.reserve(dir.size());
        for (size_t nextpos = 1; nextpos != std::string::npos;) {
            nextpos = dir.find('/',nextpos);
            const std::string* wk = &work;
            if (nextpos != std::string::npos) {
                work.assign(dir,0,nextpos);
                nextpos += 1;
            } else {
                wk = &dir;
            }
            if (-1 != mkdir(wk->c_str(),0777)) {
                smsc_log_debug(log_,"directory '%s' created",wk->c_str());
            } else if ( errno == EEXIST ) {
                smsc_log_debug(log_,"directory '%s' already exist, ok",wk->c_str());
            } else {
                const size_t buflen = 100;
                char buf[buflen];
                strerror_r(errno,buf,buflen);
                throw InfosmeException("cannot create dir '%s': %d, %s",wk->c_str(),errno,buf);
            }
        }
    }

private:
    static smsc::logger::Logger* log_;
    int fd_;
};

} // informer
} // smsc

#endif
