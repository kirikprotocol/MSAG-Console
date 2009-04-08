#ifndef _SCAG_UTIL_STORAGE_FIXUPLOGGER_H
#define _SCAG_UTIL_STORAGE_FIXUPLOGGER_H

namespace scag {
namespace util {
namespace storage {

class FixupLogger
{
private:
    enum {
        TIMESTAMP = 1
    };

public:
    FixupLogger( const std::string& path ) : path_(path)
    {
        try {
            file_.RWOpen(path_.c_str());
        } catch (smsc::core::buffers::FileException& e) {
            file_.RWCreate(path_.c_str());
        }
        file_.Seek(0,SEEK_END);
        // writing a mark
        int32_t timeMark = int32_t(time(0));
        makeComment(TIMESTAMP, sizeof(timeMark), &timeMark);
    }


    void makeComment( unsigned commentType, size_t datasize, const void* data )
    {
        save(-off_t(commentType),datasize,data);
    }


    void save( const off_t offset, size_t datasize, const void* data )
    {
        // logging the buffer
        file_.WriteNetInt32(uint32_t(datasize));
        file_.WriteNetInt64(offset);
        file_.Write(data,datasize);
        file_.WriteNetInt32(uint32_t(datasize));
    }

private:
    std::string               path_;
    smsc::core::buffers::File file_;
};

} // namespace storage
} // namespace util
} // namespace scag


namespace scag2 {
namespace util {
namespace storage {
using namespace scag::util::storage;
} // namespace storage
} // namespace util
} // namespace scag

#endif /* !_SCAG_UTIL_STORAGE_FIXUPLOGGER_H */
