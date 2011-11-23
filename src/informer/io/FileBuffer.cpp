#include "FileBuffer.h"
#include "Typedefs.h"

namespace eyeline {
namespace informer {

const char* FileBuffer::peekChar( size_t maxsize )
{
    if ( rpos_ < buf_.getPos() ) {
        return buf_.get() + rpos_;
    }
    // shifting
    buf_.setPos(0);
    rpos_ = 0;
    size_t capacity = buf_.getSize() - buf_.getPos();
    if ( maxsize > capacity ) { capacity = maxsize; }
    size_t wasread = fg_.read(buf_.get(),capacity);
    if (!wasread) {
        return 0;
    }
    buf_.setPos(wasread);
    return buf_.get();
}

char* FileBuffer::readline( size_t maxsize, size_t* prevpos )
{
    size_t scan = rpos_;
    while ( true ) {

        const size_t avail = buf_.getPos() - scan;
        if ( avail == 0 ) {

            // more data needed
            size_t capacity = buf_.getSize() - buf_.getPos();
            if ( !capacity ) {
                // FIXME: resize is needed
                if ( buf_.getSize() - rpos_ >= maxsize ) {
                    // maxsize reached
                    return 0;
                }
                if ( rpos_ > 0 ) {
                    // shifting the rest of buffer
                    for ( char *i = buf_.get()+rpos_, *ie = buf_.get()+buf_.getSize(), *o = buf_.get();
                          i != ie; ++i, ++o ) {
                        *o = *i;
                    }
                    buf_.setPos(buf_.getSize()-rpos_);
                    scan -= rpos_;
                    rpos_ = 0;
                } else {
                    buf_.reserve(buf_.getSize()+buf_.getSize()/2+100);
                }
                continue; // try to read again
            }
            if ( capacity + buf_.getPos() - rpos_ > maxsize ) {
                // limit capacity
                capacity = maxsize - buf_.getPos() + rpos_;
            }
            const size_t wasread = fg_.read( buf_.getCurPtr(), capacity );
            if ( !wasread ) {
                // eof, eol is not found                
                return 0;
            }

            // search for '\n'
            char* start = buf_.getCurPtr();
            char* ptr = static_cast<char*>(memchr(start,'\n',wasread));
            if (!ptr) {
                buf_.setPos(buf_.getPos()+wasread);
                scan += wasread;
                continue;
            }

            // found
            if ( prevpos ) {
                *prevpos = getPos();
            }
            buf_.setPos(buf_.getPos()+wasread);
            rpos_ = ptr - buf_.get() + 1;
            *ptr = '\0';
            return start;
        }
        
        char* start = buf_.get() + scan;
        char* ptr = static_cast<char*>(memchr(start,'\n',avail));
        if (!ptr) {
            // not found
            scan += avail;
            continue;
        }
        if (prevpos) {
            *prevpos = getPos();
        }
        rpos_ = ptr - buf_.get() + 1;
        *ptr = '\0';
        return start;
    }
}


size_t FileBuffer::skipline() const
{
    if ( rpos_ >= buf_.getPos() ) {
        throw InfosmeException( EXC_LOGICERROR, "skipline invoked with bufpos=%llu, rpos=%llu",
                                ulonglong(buf_.getPos()), ulonglong(rpos_) );
    }
    const size_t l = strlen(buf_.get()+rpos_);
    if ( rpos_ + l >= buf_.getPos() ) {
        throw InfosmeException( EXC_LOGICERROR, "skipline invoked with non-terminated line rpos=%llu, l=%llu, bufpos=%llu",
                                ulonglong(rpos_), ulonglong(l), ulonglong(buf_.getPos()) );
    }
    return fg_.getPos() - buf_.getPos() + rpos_ + l + 1;
}

/*
const char* FileBuffer::getHead( size_t maxsize )
{
    const size_t avail = buf_.getPos() - rpos_;
    if ( avail <= maxsize ) {
        if ( buf_.getSize() > buf_.getPos() ) {
            // set trail just after buffer
            *(buf_.get()+buf_.getPos()+1) = '\0';
        } else {
            *buf_.getCurPtr() = '\0';
        }
    } else {
        *(buf_.get() + rpos_ + maxsize) = '\0';
    }
    return buf_.get() + rpos_;
}
 */

}
}
