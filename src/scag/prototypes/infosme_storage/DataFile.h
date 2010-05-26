#ifndef _SCAG_PROTOTYPES_INFOSME_DATAFILE_H
#define _SCAG_PROTOTYPES_INFOSME_DATAFILE_H

#include <sys/types.h>
#include <string>

#include "util/int.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

/// disk sequential storage for records.
/// format of the record: not specified,
/// records are separated by '\n'.
/// The storage may be in isBeingWritten state.
class DataFile
{
public:
    DataFile( const char* path );

    const std::string& getPath() const;

    /// return the number of bytes read.
    /// may return:
    ///   0 -- no more elements,
    ///  -1 -- file is being written,
    ssize_t readBuffer( char* buf, size_t bufsize, uint32_t& offset );

    /// return current position in the file.
    inline uint32_t getOffset() const {
        return offset_;
    }

    inline uint32_t getFileSize() const {
        return fsz_;
    }

    /// check if there are more messages to read.
    inline bool hasMessages() const {
        return ( offset_ < fsz_ );
    }

    inline bool isBeingWritten() const {
        return isBeingWritten_;
    }

    class Writer {
    public:
        Writer() : df_(0), fd_(-1) {}
        ~Writer() {
            detach();
        }
        void attach( DataFile& df );
        void detach();
        void writeBuffer( const char* buf, size_t bufsize );
        /// close the fd, may be opened in subsequent writeBuffer
        void close();
    private:
        DataFile* df_;
        int fd_;
    };

private:
    /// open file readonly.
    void openReadonly();

private:
    std::string fname_;
    uint32_t    offset_;  // current offset in file
    uint32_t    fsz_;     // filesize, set in ctor, and in Writer::writeBuffer
    int         fd_;      // -1 -- file is closed.
    bool        isBeingWritten_;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_DATAFILE_H */
