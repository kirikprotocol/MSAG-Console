#include <cassert>
#include "FileReader.h"
#include "FileGuard.h"

namespace eyeline {
namespace informer {

size_t FileReader::readRecords( Buf& buf, RecordReader& reader, size_t count )
{
    assert(fg_.isOpened());
    assert(buf.GetPos()==0);
    if (count==0) return 0;
    size_t total = 0;
    do {
        char* ptr = buf.get();
        size_t wasread = fg_.read( buf.GetCurPtr(), buf.getSize() - buf.GetPos() );
        if ( !wasread ) {
            // EOF
            if (ptr < buf.GetCurPtr()) {
                throw FileDataException(fg_.getPos()-buf.GetPos(),"file is garbled");
            }
            break;
        }
        buf.SetPos(buf.GetPos()+wasread);
        size_t filePos = fg_.getPos()-buf.GetPos();
        while (ptr < buf.GetCurPtr()) {
            if (reader.isStopping()) return total;
            const size_t lensize = reader.recordLengthSize();
            if (ptr + lensize > buf.GetCurPtr()) {
                break; // need more data
            }
            FromBuf fb(ptr,lensize);
            const size_t reclen = reader.readRecordLength(filePos,fb) + lensize;
            if (ptr+reclen > buf.GetCurPtr()) {
                break; // need more data
            }
            fb.setLen(reclen);
            reader.readRecordData(filePos,fb);
            ++total;
            ptr += reclen;
            filePos += reclen;
            if (--count == 0) break;
        }
        if (count==0) break;
        if (ptr>buf.get()) {
            // shifting buffer back
            char* o = buf.get();
            const char* i = ptr;
            const char* e = buf.GetCurPtr();
            for ( ; i < e; ) {
                *o++ = *i++;
            }
        } else if ( buf.GetPos() >= buf.getSize() ) {
            buf.reserve(buf.getSize()+buf.getSize()/2+100);
        }
    } while (true);
    return total;
}

}
}
