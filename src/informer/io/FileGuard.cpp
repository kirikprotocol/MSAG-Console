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
            throw ErrnoException(errno,"ropen('%s')",fn);
        }
    }
}


void FileGuard::create( const char* fn, mode_t mode, bool mkdirs, bool truncate )
{
    // create file, open for writing
    close();
    if (fn && fn[0]) {
        smsc_log_debug(log_,"create %s mkdirs=%u trunc=%u",fn,mkdirs,truncate);
        int flags = O_WRONLY|O_CREAT;
        if (truncate) flags |= O_TRUNC;
        do {
            fd_ = open(fn,flags,mode);
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
            throw ErrnoException(errno,"create('%s',mode=%o,mkdirs=%d,trunc=%d)",fn,mode,mkdirs,truncate);
        }
    }
}


size_t FileGuard::seek( off_t pos, int whence )
{
    if (fd_!=-1) {
        off_t res = lseek(fd_,pos,whence);
        if (res==off_t(-1)) {
            throw ErrnoException(errno,"seek(%lld,%d)",static_cast<long long>(pos),whence);
        }
        pos_ = res;
    }
    return pos_;
}


const struct stat& FileGuard::getStat( struct stat& st ) const
{
    if (fd_==-1) {
        throw InfosmeException(EXC_LOGICERROR,"stat failed: file is not opened");
    }
    if (-1 == fstat(fd_,&st)) {
        throw ErrnoException(errno,"fstat");
    }
    return st;
}


void FileGuard::write( const void* buf, size_t buflen, bool atomic )
{
    if (fd_==-1) {
        throw InfosmeException(EXC_LOGICERROR,"write failed: file is not opened");
    }
    while (buflen>0) {
        ssize_t written = ::write(fd_,buf,buflen);
        if (written == -1) {
            throw ErrnoException(errno,"write(buflen=%llu)",ulonglong(buflen));
        }
        buflen -= written;
        pos_ += written;
        if (atomic && buflen>0) {
            throw InfosmeException(EXC_SYSTEM,
                                   "write was not atomic: %d, buflen=%llu written=%u",
                                   fd_,ulonglong(buflen+written),written);
        }
    }
}


size_t FileGuard::read( void* buf, size_t buflen )
{
    if (fd_==-1) {
        throw InfosmeException(EXC_LOGICERROR,"read failed: file is not opened");
    }
    size_t wasread = 0;
    while (buflen>0) {
        ssize_t readlen = ::read(fd_,buf,buflen);
        if (readlen==0) break; // EOF
        else if (readlen==-1) {
            throw ErrnoException(errno,"read(buflen=%llu)",ulonglong(buflen));
        }
        pos_ += readlen;
        wasread += readlen;
        buflen -= readlen;
        buf = reinterpret_cast<char*>(buf) + readlen;
    }
    return wasread;
}


void FileGuard::unlink( const char* fname )
{
    getlog();
    if (!fname) {
        throw InfosmeException(EXC_LOGICERROR,"NULL passed to unlink");
    }
    if ( 0 == ::unlink(fname) ) {
        smsc_log_debug(log_,"unlink('%s')",fname);
    } else {
        throw ErrnoException(errno,"unlink('%s')",fname);
    }
}


void FileGuard::makedirs( const std::string& dir )
{
    getlog();
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
            throw ErrnoException(errno,"mkdir('%s')",wk->c_str());
        }
    }
}


void FileGuard::truncate( size_t pos )
{
    if (fd_!=-1) {
        if (-1 == ftruncate(fd_,pos)) {
            throw ErrnoException(errno,"truncate(pos=%llu)", ulonglong(pos));
        }
    }
}

}
}
