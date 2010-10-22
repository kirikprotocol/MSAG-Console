#include <cerrno>
#include <cstring>
#include "FileGuard.h"
#include "Typedefs.h"

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
            char buf[100];
            throw InfosmeException("cannot open '%s' for reading: %d, %s", fn, errno, STRERROR(errno,buf,sizeof(buf)));
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
            char buf[100];
            throw InfosmeException("cannot open '%s' for writing: %d, %s", fn, errno, STRERROR(errno,buf,sizeof(buf)));
        }
    }
}


size_t FileGuard::seek( size_t pos, int whence )
{
    if (fd_!=-1) {
        off_t res = lseek(fd_,off_t(pos),whence);
        if (res==off_t(-1)) {
            char ebuf[100];
            throw InfosmeException("seek(%llu,%d) failed: %d, %d, %s",ulonglong(pos),whence,fd_,errno,STRERROR(errno,ebuf,sizeof(ebuf)));
        }
        pos_ = res;
    }
    return pos_;
}


void FileGuard::write( const void* buf, size_t buflen, bool atomic )
{
    if (fd_==-1) {
        throw InfosmeException("write failed: file is not opened");
    }
    while (buflen>0) {
        ssize_t written = ::write(fd_,buf,buflen);
        if (written == -1) {
            char ebuf[100];
            throw InfosmeException("write failed: %d, %s",fd_,errno,STRERROR(errno,ebuf,sizeof(ebuf)));
        }
        buflen -= written;
        pos_ += written;
        if (atomic && buflen>0) {
            throw InfosmeException("write was not atomic: %d, buflen=%llu written=%u",
                                   fd_,ulonglong(buflen+written),written);
        }
    }
}


size_t FileGuard::read( void* buf, size_t buflen )
{
    if (fd_==-1) {
        throw InfosmeException("read failed: file is not opened");
    }
    size_t wasread = 0;
    while (buflen>0) {
        ssize_t readlen = ::read(fd_,buf,buflen);
        if (readlen==0) break; // EOF
        else if (readlen==-1) {
            char ebuf[100];
            throw InfosmeException("read failed: %d, %d, %s", fd_, errno, STRERROR(errno,ebuf,sizeof(ebuf)));
        }
        pos_ += readlen;
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
            char buf[100];
            throw InfosmeException("cannot create dir '%s': %d, %s",wk->c_str(),errno,STRERROR(errno,buf,sizeof(buf)));
        }
    }
}


void FileGuard::truncate( size_t pos )
{
    if (fd_!=-1) {
        if (-1 == ftruncate(fd_,pos)) {
            char ebuf[100];
            throw InfosmeException("truncate failed: %d, %d, %s", fd_, errno, STRERROR(errno,ebuf,sizeof(ebuf)));
        }
    }
}

}
}
