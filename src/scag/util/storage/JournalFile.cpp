#include <functional>
#include <cassert>
#include <algorithm>
#include "JournalFile.h"
#include "scag/util/PtrLess.h"
#include "scag/util/PtrDestroy.h"
#include "EndianConverter.h"

namespace {
using namespace scag2::util::storage;
struct Applier : public std::unary_function< const JournalRecord*, void >
{
    Applier( JournalStorage& store, bool takeNew = true ) :
    store_(store), takeNew_(takeNew) {}
    void operator () ( const JournalRecord* rec ) const {
        store_.applyJournalData(*rec,takeNew_);
    }
    void setState( const JournalRecord* rec ) const {
        store_.applyJournalState(*rec,takeNew_);
    }
private:
    JournalStorage& store_;
    bool            takeNew_;
};
}


namespace scag2 {
namespace util {
namespace storage {

void JournalFile::open()
{
    const std::string fn = store_.journalFileName();
    journalFile_.RWOpen( fn.c_str() );
    journalFile_.SetUnbuffered();

    // reading the header
    const size_t mhsz = store_.maxJournalHeaderSize();
    const size_t jsz = size_t(journalFile_.Size());

    size_t hsz = 0;
    if ( mhsz > 0 ) {
        journal_.resize(mhsz);
        journalFile_.Read(&journal_[0],mhsz);
        // applying
        hsz = store_.loadJournalHeader(&journal_[0]);
    }

    // loading the whole journal file
    if ( jsz > 1000000 ) {
        throw smsc::util::Exception("too big journal file: sz=%u", unsigned(jsz) );
    }

    if ( jsz <= mhsz ) {
        return;
    }

    journal_.resize(jsz);
    journalFile_.Read(&journal_[mhsz],jsz-mhsz);
    recordSerial_ = 0;

    // loading journal records
    typedef std::vector< JournalRecord* > RecordList;
    RecordList records;
    records.reserve( maxRecords_ );
    const char* endptr = &journal_[journal_.size()];
    for ( const char* iptr = &journal_[hsz]; iptr < endptr; ) {

        const std::string& jrm = store_.journalRecordMark();
        iptr = std::find( iptr, endptr, jrm[0] );
        if ( iptr+jrm.size() >= endptr ) {
            break; // eof reached
        }
        if ( ! std::equal(jrm.begin(),jrm.end(),iptr) ) {
            // not equal
            ++iptr;
            continue;
        }

        JournalRecord* jr;
        const char* optr = iptr+jrm.size();
        try {
            jr = loadJournalRecord( optr, endptr - optr );
            // store_.loadJournalRecord( optr, endptr - iptr );
        } catch (...) { jr = 0; }
        
        if ( jr ) {
            iptr = optr;
            records.push_back(jr);
            jr->setEndsAt( optr - &journal_[0] );
            // check for trailer
            const std::string& jrt = store_.journalRecordTrailer();
            if ( iptr + jrt.size() >= endptr ) {
                // file has ended
                break; 
            }
            if ( std::equal(jrt.begin(),jrt.end(),iptr) ) {
                // eof found
                break;
            }
        } else {
            ++iptr;
        }
    } // loop over file content

    // clearing journal content
    buffer_type().swap(journal_);
    
    if ( records.size() == 0 ) {
        return;
    }

    try {

        if ( records.size() > 1 ) {

            const uint32_t serialShift = ( records.front()->getSerial() > 0x40000000U &&
                                           records.front()->getSerial() < 0xc0000000U ) ?
                0U : 0x80000000U;
            for ( RecordList::const_iterator i = records.begin();
                  i != records.end();
                  ++i ) {
                (*i)->setSerial( (*i)->getSerial() + serialShift );
            }

            // check that all records have sequential serials
            std::sort( records.begin(), records.end(), PtrLess() );
            RecordList::const_iterator i = records.begin();
            for ( RecordList::const_iterator j = i+1;
                  j != records.end(); ++j ) {
                if ( (*i)->getSerial() + 1 != (*j)->getSerial() ) {
                    throw smsc::util::Exception("journal records are not sequential: %u %u",
                                                unsigned((*i)->getSerial()-serialShift),
                                                unsigned((*j)->getSerial()-serialShift));
                }
                ++i;
            }
            // shifting back
            for ( RecordList::const_iterator i = records.begin();
                  i != records.end();
                  ++i ) {
                (*i)->setSerial( (*i)->getSerial() - serialShift );
            }
        }

        // all records are sequential
        store_.prepareForApplication( records );

        try {

            // applying new states
            ::Applier applier(store_,true);
            std::for_each( records.begin(), records.end(), applier );
            applier.setState( records.back() );
            journalFile_.Seek( records.back()->getEndsAt() );
            recordSerial_ = records.back()->getSerial();

        } catch (...) {
            
            // trying with old
            ::Applier applier(store_,false);
            std::for_each( records.rbegin(), records.rend(), applier );
            applier.setState( records.front() );
            // FIXME: rename the orig file
            recordSerial_ = 0;
        }

    } catch ( ... ) {
        std::for_each( records.begin(), records.end(), PtrDestroy() );
        throw;
    }
    std::for_each( records.begin(), records.end(), PtrDestroy() );
}


void JournalFile::create()
{
    const std::string fn = store_.journalFileName();
    if ( File::Exists(fn.c_str()) ) {
        throw smsc::util::Exception("file %s already exists", fn.c_str());
    }
    journalFile_.RWCreate( fn.c_str() );
    journalFile_.SetUnbuffered();
    recordSerial_ = 0;
}


void JournalFile::recreate()
{
    const std::string fn = store_.journalFileName();
    if ( File::Exists(fn.c_str()) ) {
        File::Rename(fn.c_str(),(fn+".bak").c_str());
    }
    create();
}


bool JournalFile::writeRecord( const JournalRecord& record )
{
    const bool needHeader = ( recordSerial_ == 0 );
    ++recordSerial_;
    if ( recordSerial_ == 0 ) { ++recordSerial_; }
    const bool lastRecord = ( recordSerial_ % maxRecords_ ) == 0;

    const uint32_t jnlSize = constantRecordSize() + record.savedDataSize();
    journal_.resize( ( needHeader ? store_.journalHeaderSize() : 0 ) +
                     store_.journalRecordMark().size() +
                     jnlSize +
                     ( lastRecord ? store_.journalRecordTrailer().size() : 0 ) );
    char* ptr = &journal_[0];
    if ( needHeader && store_.journalHeaderSize() > 0 ) {
        store_.saveJournalHeader(ptr);
        ptr += store_.journalHeaderSize();
    }

    uint64_t csum;
    do {
        csum = rnd_.getNextNumber();
    } while ( csum == 0 );
    ptr = mempcpy(ptr,store_.journalRecordMark().c_str(),store_.journalRecordMark().size());
    const char* ptr0 = ptr; // remember position
    EndianConverter::set32(ptr,jnlSize);
    ptr = mempcpy(ptr+4,&csum,8);
    EndianConverter::set32(ptr,recordSerial_);
    ptr += 4;
    record.save(ptr);
    ptr += record.savedDataSize();
    ptr = mempcpy(ptr,ptr0+4,8); // csum again
    ptr = mempcpy(ptr,ptr0,4);   // size again
    if ( lastRecord ) {
        ptr = mempcpy(ptr,store_.journalRecordTrailer().c_str(),store_.journalRecordTrailer().size());
    }
    assert( ptr == &journal_[journal_.size()] );
    if ( needHeader ) { journalFile_.Seek(0); }
    journalFile_.Write( &journal_[0], journal_.size() );
    if ( lastRecord ) {
        journalFile_.Truncate(journalFile_.Pos());
        journalFile_.Seek( store_.journalHeaderSize() );
    }
    return lastRecord;
}


JournalRecord* JournalFile::loadJournalRecord( const char*& iptr, size_t bufsize )
{
    if ( bufsize <= constantRecordSize() ) {
        throw smsc::util::Exception("the buffer is too small %u, must be at least %u",
                                    unsigned(bufsize), unsigned(constantRecordSize()));
    }
    // loading size
    const char* ptr = iptr;
    const uint32_t jnlSize = EndianConverter::get32(ptr);
    const char* endptr = ptr+jnlSize;
    if ( bufsize < jnlSize ) {
        throw smsc::util::Exception("record size %u, buffer size is too small = %u",
                                    unsigned(jnlSize), unsigned(bufsize));
    }
    // checking control sums, etc.
    if ( ! std::equal(ptr,ptr+4,endptr-4) ) {
        throw smsc::util::Exception("record sizes do not match");
    }
    ptr += 4;
    if ( ! std::equal(ptr,ptr+8,endptr-12) ) {
        throw smsc::util::Exception("control sums do not match");
    }
    std::auto_ptr< JournalRecord > jr( store_.createJournalRecord() );
    jr->setControlSum( EndianConverter::get64(ptr) );
    jr->setSerial( EndianConverter::get32(ptr+8) );
    jr->load(ptr+12,jnlSize-constantRecordSize());
    iptr = endptr;
    return jr.release();
}

} // namespace storage
} // namespace util
} // namespace scag2
