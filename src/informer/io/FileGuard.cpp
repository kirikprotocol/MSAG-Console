#include <cerrno>
#include <cstring>
#include "FileGuard.h"
#include "Typedefs.h"
#include "DirListing.h"

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
    if ( dir.empty() ) {
        throw InfosmeException(EXC_LOGICERROR,"mkdir('') is incorrect");
    }
    getlog();
    // first of all try to make the last dir only
    if ( -1 != mkdir(dir.c_str(),0777)) {
        smsc_log_debug(log_,"directory '%s' created",dir.c_str());
        return;
    } else if ( errno == EEXIST ) {
        // smsc_log_debug(log_,"directory '%s' already exist, ok",dir.c_str());
        return;
    }
    struct stat dst;
    std::string work;
    work.reserve(dir.size()+1);
    for (size_t nextpos = 1; nextpos != std::string::npos;) {
        nextpos = dir.find('/',nextpos);
        const std::string* wk = &work;
        if (nextpos != std::string::npos) {
            work.assign(dir,0,nextpos);
            nextpos += 1;
        } else {
            wk = &dir;
        }
        if ( -1 == stat(wk->c_str(),&dst) ) {
            if ( errno == ENOENT ) {
                // not existing
                if (-1 != mkdir(wk->c_str(),0777)) {
                    smsc_log_debug(log_,"directory '%s' created",wk->c_str());
                } else if ( errno == EEXIST ) {
                    // smsc_log_debug(log_,"directory '%s' already exist, ok",wk->c_str());
                } else {
                    throw ErrnoException(errno,"mkdir('%s')",wk->c_str());
                }
            } else {
                throw ErrnoException(errno,"stat('%s')",wk->c_str());
            }
        } else if ( !S_ISDIR(dst.st_mode) ) {
            throw InfosmeException(EXC_LOGICERROR,"path '%s' exist and not a dir",wk->c_str());
        }
    }
}


void FileGuard::rmdirs( const char* path, bool rmself )
{
    getlog();
    smsc_log_debug(log_,"rmdirs('%s',self=%u)",path,rmself);
    std::vector< std::string > contents;
    makeDirListing( NoDotsNameFilter() ).list( path, contents );
    if ( !contents.empty() ) {
        smsc::core::buffers::TmpBuf<char,256> fpath;
        const size_t pathlen = strlen(path);
        if (path[pathlen-1] != '/') {
            throw InfosmeException(EXC_LOGICERROR,"rmdirs('%s') should ends with /",path);
        }
        fpath.setSize(pathlen+30);
        strcpy(fpath.get(),path);
        for ( std::vector<std::string>::const_iterator i = contents.begin();
              i != contents.end(); ++i ) {
            fpath.SetPos(pathlen);
            fpath.Append(i->c_str(),i->size()+1);
            struct stat st;
            if ( -1 == ::lstat(fpath.get(),&st) ) {
                throw ErrnoException(errno,"lstat('%s')",fpath.get());
            }
            if ( S_ISDIR(st.st_mode) ) {
                // directory, must be deleted recursively
                fpath.SetPos(fpath.GetPos()-1);
                fpath.Append("/",2);
                rmdirs(fpath.get(),true);
            } else {
                if ( -1 == ::unlink(fpath.get()) ) {
                    throw ErrnoException(errno,"unlink('%s')",fpath.get());
                }
            }
        }
    }
    if (rmself) {
        if ( -1 == ::rmdir(path) ) {
            throw ErrnoException(errno,"rmdir('%s')",path);
        }
    }
}


void FileGuard::copydir( const char* from,
                         const std::string& to,
                         unsigned maxdepth )
{
    getlog();
    smsc_log_debug(log_,"copydir('%s','%s')",from,to.c_str());
    struct stat st;
    if ( -1 == ::stat(from,&st) ) {
        throw InfosmeException(EXC_SYSTEM,"copydir('%s') does not exist",from);
    } else if ( !S_ISDIR(st.st_mode) ) {
        throw InfosmeException(EXC_SYSTEM,"copydir('%s') is not a dir",from);
    }
    makedirs(to);
    std::vector< std::string > contents;
    makeDirListing( NoDotsNameFilter() ).list(from,contents);
    if ( ! contents.empty() && maxdepth > 0 ) {
        // copying contents
        smsc::core::buffers::TmpBuf<char,256> fpath;
        const size_t fromlen = strlen(from);
        if ( from[fromlen-1] != '/' ) {
            throw InfosmeException(EXC_LOGICERROR,"copydir('%s') should ends with /",from);
        }
        fpath.setSize(fromlen+30);
        strcpy(fpath.get(),from);
        for ( std::vector< std::string >::const_iterator i = contents.begin();
              i != contents.end(); ++i ) {
            fpath.SetPos(fromlen);
            fpath.Append( i->c_str(), i->size()+1 );
            if ( -1 == ::stat(fpath.get(),&st) ) {
                throw ErrnoException(errno,"stat('%s')",fpath.get());
            }
            if ( S_ISDIR(st.st_mode) ) {
                fpath.SetPos(fpath.GetPos()-1);
                fpath.Append("/",2);
                copydir(fpath.get(), to + *i + "/", maxdepth-1);
            } else if ( S_ISREG(st.st_mode) ) {
                copyfile(fpath.get(), (to+*i).c_str());
            }
        }
    }
}


void FileGuard::copyfile( const char* from,
                          const char* to )
{
    FileGuard fg, fd;
    fg.ropen(from);
    struct stat st;
    fd.create(to, fg.getStat(st).st_mode & (S_IRWXU|S_IRWXG|S_IRWXO),
              true, true );
    char buf[8192];
    while ( true ) {
        const size_t wasread = fg.read(buf,sizeof(buf));
        if (wasread == 0) { break; }
        fd.write(buf,wasread);
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


void FileGuard::close( bool dosync )
{
    if (fd_!=-1) {
        if ( dosync && (0 != ::fsync(fd_)) ) {
            throw ErrnoException(errno,"fsync fd=%d",fd_);
        }
        if ( 0 != ::close(fd_) ) {
            throw ErrnoException(errno,"fsync fd=%d",fd_);
        }
    }
    fd_ = -1;
    pos_ = 0;
}

}
}
