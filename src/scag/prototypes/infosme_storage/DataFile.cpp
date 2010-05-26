#include <sys/stat.h>
#include <fcntl.h>

#include "DataFile.h"
#include "util/Exception.hpp"

namespace scag2 {
namespace prototypes {
namespace infosme {

ssize_t DataFile::readBuffer( char* buf, size_t bufsize, uint32_t& offset )
{
    if ( isBeingWritten() && offset_ + 512 >= fsz_ ) {
        // not ready
        return -1;
    }
    if ( offset_ >= fsz_ ) {
        // eof
        close(fd_);
        fd_ = -1;
        return 0;
    }
    if (fd_ == -1) {
        openReadonly();
    }
    offset = offset_;
    const ssize_t ret = read(fd_,buf,bufsize);
    if (ret==-1) {
        throw smsc::util::Exception("cannot read %s: %u",fname_.c_str(),errno);
    } else if (ret==0) {
        /// eof?
        return isBeingWritten() ? -1 : 0;
    }
    return ret;
}


void DataFile::openReadonly()
{
    if ( fd_ != -1 ) return; // already opened
    fd_ = open(fname_.c_str(),O_RDONLY);
    if ( fd_ == -1 ) {
        throw smsc::util::Exception("cannot open file %s for reading: %d",fname_.c_str(),errno);
    }
    if ( offset_ > 0 ) {
        const off_t curoff = lseek(fd_,offset_,SEEK_SET);
        if ( curoff == off_t(-1) ) {
            close(fd_);
            fd_ = -1;
            throw smsc::util::Exception("cannot seek %s to pos %u: %d",fname_.c_str(),offset_,errno);
        } else if ( curoff != offset_ ) {
            close(fd_);
            fd_ = -1;
            throw smsc::util::Exception("seek %s to pos %u returned different value %u",fname_.c_str(),offset_,unsigned(curoff));
        }
    }
}

// ========================= writer

void DataFile::Writer::attach( DataFile& df )
{
    if (df_) {
        if (df_ != &df) throw smsc::util::Exception("Writer is already attached to %s",df_->getPath().c_str());
    } else {
        if (df.isBeingWritten()) throw smsc::util::Exception("DataFile %s is already being written",df.getPath().c_str());
        df_ = &df;
        df_->isBeingWritten_ = true;
    }
}


void DataFile::Writer::detach()
{
    if (df_) {
        df_->isBeingWritten = false;
        df_ = 0;
    }
    this->close();
}


void DataFile::Writer::writeBuffer( const char* buf, size_t bufsize )
{
    if (!df_) throw smsc::util::Exception("writer is not attached");
    if (fd_==-1) {
        fd_ = open(df_.getPath().c_str(),O_WRONLY|O_CREAT,0777);
        if (fd_==-1) {
            throw smsc::util::Exception("cannot open file %s for writing: %d",
                                        df_->getPath().c_str(),errno);
        }
        if ( lseek(fd_,0,SEEK_END) == off_t(-1) ) {
            close(fd_);
            fd_ = -1;
            throw smsc::util::Exception("cannot seek to the end of %s: %d",
                                        df_->getPath().c_str(),errno);
        }
    }
    const size_t bsz = bufsize;
    while ( bufsize > 0 ) {
        ssize_t written = write(fd_,buf,bufsize);
        if (written==-1) {
            throw smsc::util::Exception("cannot write to file %s: %d",
                                        df_->getPath().c_str(),errno);
        } else if (written == 0) {
            timespec sleepNs = {0,10000};
            nanosleep(&sleepNs,0);
        } else {
            bufsize -= written;
            buf += written;
        }
    }
    df_->fsz_ += bsz;
}


void DataFile::Writer::close()
{
    if (fd_!=-1) {
        close(fd_);
        fd_ = -1;
    }
}

}
}
}
