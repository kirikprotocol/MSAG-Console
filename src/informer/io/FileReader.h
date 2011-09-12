#ifndef _INFORMER_FILEREADER_H
#define _INFORMER_FILEREADER_H

#include "TmpBuf.h"
#include "InfosmeException.h"
#include "FileGuard.h"
#include "Typedefs.h"
// #include "IOConverter.h"

namespace eyeline {
namespace informer {

class FileGuard;

class FileReader
{
public:
    struct RecordReader {
        /// check if reader is stopping
        virtual bool isStopping() = 0;

        /// return the size of record length in octets.
        /// for special-char-terminated records please return 0.
        virtual size_t recordLengthSize() const = 0;

        /// read record length from fb and checks its validity.
        /// for special-char-terminated records please find out it from the stream.
        /// if the char is not found return buflen+1.
        virtual size_t readRecordLength( size_t filePos,
                                         char* buf,
                                         size_t buflen ) = 0;

        /// read the record data (w/o length)
        /// @return false if the record is filtered.
        /// @param filePos -- is the position of the beginning of the record (including length)
        virtual bool readRecordData( size_t filePos,
                                     char* buf,
                                     size_t buflen ) = 0;
    };

    FileReader( FileGuard& fg ) : fg_(fg) {}

    /// read records via reader and return the number of records read.
    size_t readRecords( TmpBufBase<char>& buf,
                        RecordReader& reader,
                        size_t count = size_t(0xffffffffffffffffULL) );

private:
    FileGuard& fg_;
};

} // informer
} // smsc

#endif
