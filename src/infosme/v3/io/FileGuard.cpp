#include "FileGuard.h"

namespace eyeline {
namespace informer {

smsc::logger::Logger* FileGuard::log_ = 0;


void FileGuard::ropen( const char* fn )
{
    // open file readonly
    close();
    if (fn && fn[0]) {
        smsc_log_debug(log_,"ropen %s",fn);
        fd_ = open(fn,O_RDONLY);
        if (fd_==-1) {
            const size_t buflen = 100;
            char buf[buflen];
            strerror_r(errno,buf,buflen);
            throw InfosmeException("cannot open '%s' for reading: %d, %s", fn, errno, buf);
        }
    }
}


void FileGuard::create( const char* fn, bool mkdirs, bool truncate )
{
    // create file, open for writing
    close();
    if (fn && fn[0]) {
        smsc_log_debug(log_,"create %s mkdirs=%u trunc=%u",fn,mkdirs,truncate);
        int flags = O_WRONLY|O_CREAT;
        if (truncate) flags |= O_TRUNC;
        do {
            fd_ = open(fn,flags,0777);
            if (fd_!= -1) break; // ok
            if (!mkdirs) break;  // dir creation is not allowed
            if (errno!=ENOENT) break; // other problems
            // trying to create directory
            std::string dir;
            const char* p = strrchr(fn,'/');
            if (p) {
                dir.assign(fn,p-fn);
            } else {
                dir = ".";
            }
            makedirs( dir );
            mkdirs = false;
        } while (true);

        if (fd_==-1) {
            const size_t buflen = 100;
            char buf[buflen];
            strerror_r(errno,buf,buflen);
            throw InfosmeException("cannot open '%s' for writing: %d, %s", fn, errno, buf);
        }
    }
}


void FileGuard::write( const void* buf, unsigned buflen, bool atomic )
{
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
        if (atomic && buflen>0) {
            const size_t ebuflen = 100;
            char ebuf[ebuflen];
            strerror_r(errno,ebuf,ebuflen);
            throw InfosmeException("write was not atomic: %d, buflen=%u written=%u",
                                   fd_,buflen+written,written);
        }
    }
}


unsigned FileGuard::read( void* buf, unsigned buflen )
{
    if (fd_==-1) {
        throw InfosmeException("read failed: file is not opened");
    }
    unsigned wasread = 0;
    while (buflen>0) {
        ssize_t readlen = ::read(fd_,buf,buflen);
        if (readlen==0) break; // EOF
        else if (readlen==-1) {
            const size_t ebuflen = 100;
            char ebuf[ebuflen];
            strerror_r(errno,ebuf,ebuflen);
            throw InfosmeException("read failed: %d, %d, %s", fd_, errno, ebuf);
        }
        wasread += readlen;
        buflen -= readlen;
        buf = reinterpret_cast<char*>(buf) + readlen;
    }
    return wasread;
}


void FileGuard::makedirs( const std::string& dir )
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

}
}
