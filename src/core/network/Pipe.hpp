#ifndef __CORE_NETWORK_PIPE_HPP__
#define __CORE_NETWORK_PIPE_HPP__

#include <unistd.h>
#include "util/Exception.hpp"

namespace smsc {
namespace core {
namespace network {

/// the pipe, may be used e.g. for waking up Multiplexer
class Pipe
{
public:
    Pipe() {
        if ( -1 == pipe(fds_) ) {
            fds_[0] = fds_[1] = -1;
        }
    }

    ~Pipe() {
        close();
    }
    
    int getR() { return fds_[0]; }

    int getW() { return fds_[1]; }

    /// may be used to close one side of the pipe
    void close( int what = 3 ) {
        if ((what & 2) && fds_[1] != -1) { ::close(fds_[1]); fds_[1] = -1; }
        if ((what & 1) && fds_[0] != -1) { ::close(fds_[0]); fds_[0] = -1; }
    }

    void write( const void* buf, size_t sz ) {
        if ( fds_[1] == -1 ) throw smsc::util::Exception("pipe.w is not opened");
        const ssize_t wsz = ::write(fds_[1],buf,sz);
        if ( -1 == wsz ) throw smsc::util::Exception("pipe write failed: errno=%d",errno);
        if ( wsz != ssize_t(sz) ) throw smsc::util::Exception("pipe wsz=%ld less than sz=%ld",
                                                              long(wsz), long(sz));
    }
    
    size_t read( void* buf, size_t sz ) {
        if ( fds_[0] == -1 ) throw smsc::util::Exception("pipe.r is not opened");
        const ssize_t rsz = ::read(fds_[0],buf,sz);
        if ( -1 == rsz ) throw smsc::util::Exception("pipe read failed: errno=%d",errno);
        return size_t(rsz);
    }

private:
    int fds_[2];
};

}
}
}

#endif
