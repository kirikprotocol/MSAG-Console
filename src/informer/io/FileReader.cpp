#include <cassert>
#include "FileReader.h"
#include "FileGuard.h"

namespace {
smsc::logger::Logger* log_ = 0;
}


namespace eyeline {
namespace informer {

size_t FileReader::readRecords( Buf& buf, RecordReader& reader, size_t count )
{
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("fileread");
    }
    assert(fg_.isOpened());
    assert(buf.GetPos()==0);
    if (count==0) return 0;
    smsc_log_debug(log_,"trying to read %llu records",ulonglong(count));
    size_t total = 0;
    do {
        char* ptr = buf.get();
        const size_t wasread = fg_.read( buf.GetCurPtr(), buf.getSize() - buf.GetPos() );
        smsc_log_debug(log_,"read %llu bytes, bufpos=%u, filepos=%llu",
                       ulonglong(wasread),
                       unsigned(buf.GetPos()),
                       ulonglong(fg_.getPos()));
        if ( !wasread ) {
            // EOF
            smsc_log_debug(log_,"EOF found");
            if (ptr < buf.GetCurPtr()) {
                throw FileDataException(fg_.getPos()-buf.GetPos(),"file is garbled");
            }
            break;
        }
        buf.SetPos(buf.GetPos()+wasread);
        size_t filePos = fg_.getPos()-buf.GetPos();
        while (ptr < buf.GetCurPtr()) {
            if (reader.isStopping()) return total;
            smsc_log_debug(log_,"next try pos=%u buflen=%u",
                           unsigned(ptr-buf.get()), unsigned(buf.GetPos()));
            const size_t lensize = reader.recordLengthSize();
            if (ptr + lensize > buf.GetCurPtr()) {
                break; // need more data
            }
            FromBuf fb(ptr,lensize);
            const size_t reclen = reader.readRecordLength(filePos,fb) + lensize;
            smsc_log_debug(log_,"reclen=%u",unsigned(reclen));
            if (ptr+reclen > buf.GetCurPtr()) {
                break; // need more data
            }
            smsc_log_debug(log_,"processing record=%u len=%u at pos=%u, fpos=%llu",
                           unsigned(total),
                           unsigned(reclen),
                           unsigned(ptr-buf.get()),
                           ulonglong(filePos));
            fb.setLen(reclen);
            if (reader.readRecordData(filePos,fb)) {
                ++total;
            }
            ptr += reclen;
            filePos += reclen;
            if (total >= count) break;
        }
        if (total >= count) break;
        if (ptr>buf.get()) {
            // shifting buffer back
            smsc_log_debug(log_,"shifting %u bytes from pos=%u to 0",
                           unsigned(buf.GetCurPtr()-ptr), unsigned(ptr-buf.get()));
            char* o = buf.get();
            const char* i = ptr;
            const char* e = buf.GetCurPtr();
            for ( ; i < e; ) {
                *o++ = *i++;
            }
            buf.SetPos(buf.GetCurPtr()-ptr);
        } else if ( buf.GetPos() >= buf.getSize() ) {
            buf.reserve(buf.getSize()+buf.getSize()/2+100);
        }
    } while (true);
    return total;
}

}
}
