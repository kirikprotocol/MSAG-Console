#ifndef _INFORMER_FILEBUFFER_H
#define _INFORMER_FILEBUFFER_H

#include "FileGuard.h"
#include "TmpBuf.h"

namespace eyeline {
namespace informer {

class FileBuffer
{
public:
    FileBuffer( FileGuard& fg, TmpBufBase<char>& buf ) :
    fg_(fg), buf_(buf), rpos_(0) {
        buf_.setPos(0);
    }

    // get current position in file
    inline size_t getPos() const {
        return fg_.getPos() - buf_.getPos() + rpos_;
    }

    // get file size
    inline size_t getFileSize() const {
        typedef struct stat mystat;
        mystat st;
        return fg_.getStat(st).st_size;
    }

    // peek a char, w/o shifting position
    // @return ptr to char or 0 if at end.
    const char* peekChar( size_t sizehint );

    // get current line and advance position
    // @param if successful then is set to position of linestart.
    // @return line pointer or 0
    char* readline( size_t maxsize, size_t* prevpos );

    // return the head of the buffer, limiting its length at maxsize
    // NOTE: the buffer content is broken.
    // const char* getHead( size_t maxsize );

    size_t skipline() const;

    inline void setPos( size_t pos, bool reset = false ) {
        if ( ( pos < fg_.getPos() - buf_.getPos() ) ||
             ( pos > fg_.getPos() ) || reset ) {
            // out of buffer position
            fg_.seek(pos);
            buf_.setPos(0);
            rpos_ = 0;
        } else {
            rpos_ = pos - (fg_.getPos() - buf_.getPos());
        }
    }

private:
    FileGuard&        fg_;
    TmpBufBase<char>& buf_;
    size_t            rpos_;
};

} // informer
} // smsc

#endif
