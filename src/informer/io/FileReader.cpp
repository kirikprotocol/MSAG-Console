#include <cassert>
#include "FileReader.h"
#include "FileGuard.h"

namespace {
smsc::logger::Logger* log_ = 0;
}


namespace eyeline {
namespace informer {

size_t FileReader::readRecords( TmpBufBase<char>& buf,
                                RecordReader& reader,
                                size_t count )
{
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("fileread");
    }
    assert(fg_.isOpened());
    assert(buf.getPos()==0);
    if (count==0) return 0;
    smsc_log_debug(log_,"trying to read %llu records",ulonglong(count));
    size_t total = 0;
    do {
        char* ptr = buf.get();
        const size_t wasread = fg_.read( buf.getCurPtr(), buf.getSize() - buf.getPos() );
        smsc_log_debug(log_,"read %llu bytes, bufpos=%u, filepos=%llu",
                       ulonglong(wasread),
                       unsigned(buf.getPos()),
                       ulonglong(fg_.getPos()));
        if ( !wasread ) {
            // EOF
            smsc_log_debug(log_,"EOF found");
            if (ptr < buf.getCurPtr()) {
                throw FileDataException(fg_.getPos()-buf.getPos(),"file is garbled");
            }
            break;
        }
        buf.setPos(buf.getPos()+wasread);
        size_t filePos = fg_.getPos()-buf.getPos();
        while (ptr < buf.getCurPtr()) {
            if (reader.isStopping()) return total;
            smsc_log_debug(log_,"next try pos=%u buflen=%u",
                           unsigned(ptr-buf.get()), unsigned(buf.getPos()));
            const size_t lensize = reader.recordLengthSize();
            if (ptr + lensize > buf.getCurPtr()) {
                break; // need more data
            }
            FromBuf fb(ptr,lensize ? lensize : buf.getCurPtr()-ptr);
            const size_t reclen = reader.readRecordLength(filePos,fb) + lensize;
            smsc_log_debug(log_,"reclen=%u",unsigned(reclen));
            if (ptr+reclen > buf.getCurPtr()) {
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
                           unsigned(buf.getCurPtr()-ptr), unsigned(ptr-buf.get()));
            char* o = buf.get();
            const char* i = ptr;
            const char* e = buf.getCurPtr();
            for ( ; i < e; ) {
                *o++ = *i++;
            }
            buf.setPos(buf.getCurPtr()-ptr);
        } else if ( buf.getPos() >= buf.getSize() ) {
            buf.reserve(buf.getSize()+buf.getSize()/2+100);
        }
    } while (true);
    return total;
}

}
}
