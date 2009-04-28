#ifndef _SCAG_UTIL_STORAGE_JOURNALFILE_H
#define _SCAG_UTIL_STORAGE_JOURNALFILE_H

#include <vector>
#include <cstring>
#include "util/int.h"
#include "logger/Logger.h"
#include "core/buffers/File.hpp"
#include "scag/util/Drndm.h"

namespace scag2 {
namespace util {
namespace storage {

/// abstract journal record.
/// Journal record has two sides (old and new).
/// On each side it has a state which determines the overall state of storage
/// and data which determine the location and value of data in storage
/// which were changed during the transaction.
/// Also it has a serial number.
class JournalRecord
{
public:
    virtual ~JournalRecord() {}

    inline uint32_t getSerial() const { return serial_; }
    inline void     setSerial( uint32_t ser ) { serial_ = ser; }
    inline size_t   getEndsAt() const { return endsAt_; }
    inline void     setEndsAt( size_t pos ) { endsAt_ = pos; }
    inline uint64_t getControlSum() const { return controlSum_; }
    inline void     setControlSum( uint64_t cs ) { controlSum_ = cs; }

    /// NOTE: it is guaranteed that buffer will live longer than record.
    virtual void load( const void* buf, size_t bufSize ) = 0;

    /// not including headers and trailers
    virtual size_t savedDataSize() const = 0;
    virtual void save( void* buf ) const = 0;

    bool operator < ( const JournalRecord& jr ) const { return serial_ < jr.serial_; }

private:
    uint32_t serial_;
    size_t   endsAt_;
    uint64_t controlSum_;
};


/// interface to a journalled storage
class JournalStorage
{
public:
    virtual ~JournalStorage() {}

    /// --- common methods

    /// create a journal file name
    virtual std::string journalFileName() const = 0;

    /// the actual size of journal header (used for writing)
    virtual size_t journalHeaderSize() const = 0;

    /// the maximal size of journal header.
    /// The buffer of this size will be passed to loadJournalHeader.
    virtual size_t maxJournalHeaderSize() const = 0;

    /// these are character constants that delimit journal records in journal file
    virtual const std::string& journalRecordMark() const = 0;
    virtual const std::string& journalRecordTrailer() const = 0;

    /// --- methods for storing into journal file
    virtual void saveJournalHeader( void* buffer ) const = 0; // no throw

    /// --- methods for loading from existing jnl file

    /// load a journal header.
    /// @return actual size of header read
    virtual size_t loadJournalHeader( const void* buffer ) = 0; // throw

    /// creates an empty record
    virtual JournalRecord* createJournalRecord() = 0; // no throw

    /// preparation for application of sorted records.
    virtual void prepareForApplication( const std::vector< JournalRecord* >& records ) = 0;

    /// applying a journal record data to storage (old/new).
    virtual void applyJournalData( const JournalRecord& rec,
                                   bool takeNew ) = 0; // throw

    /// applying a journal record state to storage (old/new).
    virtual void applyJournalState( const JournalRecord& rec,
                                    bool takeNew ) = 0; // throw

};



/// journal file
class JournalFile
{
private:
    typedef std::vector< char > buffer_type;

public:
    JournalFile( JournalStorage& store,
                 uint32_t maxRecords = 100,
                 smsc::logger::Logger* logger = 0 ) :
    log_(logger), store_(store), recordSerial_(0), maxRecords_(maxRecords) {
        rnd_.setSeed( time(0) );
    }

    /// opens journal file
    void open(); // throw

    /// create a new journal file
    void create(); // throw

    /// move old journal to .bak, then create a new file
    void recreate(); // throw

    /// write a new record,
    /// return true, if this record is the last in journal file (file is wrapped).
    /// NOTE: serialized record is valid in memory until the next call to
    /// any of the modifying methods.
    bool writeRecord( const JournalRecord& record ); // throw

private:
    JournalRecord* loadJournalRecord( const char*& ptr,
                                      size_t       bufsize ); // throw

    inline size_t constantRecordSize() const {
        static const size_t crs =
            4*2 +   // 2 len
            8*2 +   // 2 csum
            4;      // serial
        return crs;
    }

    inline char* mempcpy( char* optr, const void* iptr, size_t sz ) {
        memcpy(optr,iptr,sz);
        return optr+sz;
    }

private:
    smsc::logger::Logger*      log_;
    JournalStorage&            store_;
    smsc::core::buffers::File  journalFile_;
    uint32_t                   recordSerial_;
    uint32_t                   maxRecords_;
    buffer_type                journal_;
    Drndm                      rnd_;
};

} // namespace storage
} // namespace util
} // namespace scag2

namespace scag {
namespace util {
namespace storage {
using namespace scag2::util::storage;
} // namespace storage
} // namespace util
} // namespace scag

#endif /* !_SCAG_UTIL_STORAGE_JOURNALFILE_H */
