#ifndef _INFORMER_FILEREADER_H
#define _INFORMER_FILEREADER_H

#include "core/buffers/TmpBuf.hpp"
#include "InfosmeException.h"
#include "FileGuard.h"
#include "Typedefs.h"
#include "IOConverter.h"

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
        virtual size_t recordLengthSize() const = 0;
        /// read record length from fb and checks its validity.
        virtual size_t readRecordLength( size_t filePos, FromBuf& fb ) = 0;
        /// read the record data (w/o length)
        /// @return false if the record is filtered.
        virtual bool readRecordData( size_t filePos, FromBuf& fb ) = 0;
    };

    typedef smsc::core::buffers::TmpBuf<char,8192> Buf;

    FileReader( FileGuard& fg ) : fg_(fg) {}

    /// read records via reader and return the number of records read.
    size_t readRecords( Buf& buf, RecordReader& reader, size_t count = size_t(0xffffffffffffffffULL) );

private:
    FileGuard& fg_;
};

} // informer
} // smsc

#endif
