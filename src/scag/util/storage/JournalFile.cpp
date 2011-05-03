#include <functional>
#include <cassert>
#include <algorithm>
#include <sstream>
#include "JournalFile.h"
#include "util/PtrLess.h"
#include "util/PtrDestroy.h"
#include "scag/util/io/EndianConverter.h"

using namespace smsc::core::buffers;

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

void JournalFile::open( bool readonly )
{
    const std::string fn = store_.journalFileName();
    if ( readonly ) {
        journalFile_.ROpen( fn.c_str() );
    } else {
        journalFile_.RWOpen( fn.c_str() );
    }
    journalFile_.SetUnbuffered();
    if (log_) {
        smsc_log_debug(log_,"journal file %s has been opened%s",
                       fn.c_str(),
                       readonly ? " readonly" : "");
    }

    // reading the header
    const size_t mhsz = store_.maxJournalHeaderSize();
    const size_t jsz = size_t(journalFile_.Size());

    size_t hsz = 0;
    if ( mhsz > 0 ) {
        journal_.resize(mhsz);
        journalFile_.Read(&journal_[0],mhsz);
        // applying
        hsz = store_.loadJournalHeader(&journal_[0]);
        if (log_) {
            smsc_log_debug(log_,"header of size=%u has been loaded",unsigned(hsz));
        }
    }

    // loading the whole journal file
    // if ( jsz > 1000000 ) {
    // throw smsc::util::Exception("too big journal file: sz=%u", unsigned(jsz) );
    // }

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

        if (log_) {
            smsc_log_debug(log_,"journal record found at %u", unsigned(iptr-&journal_[0]));
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
            if (log_) {
                smsc_log_debug(log_,"journal record %s has been loaded",jr->toString().c_str());
            }
            // check for trailer
            const std::string& jrt = store_.journalRecordTrailer();
            if ( iptr + jrt.size() >= endptr ) {
                // file has ended
                break; 
            }
            if ( std::equal(jrt.begin(),jrt.end(),iptr) ) {
                // eof found
                if (log_) {
                    smsc_log_debug(log_,"trailer found at %u", unsigned(iptr-&journal_[0]));
                }
                break;
            }
        } else {
            ++iptr;
        }
    } // loop over file content

    if ( records.size() == 0 ) {
        if (log_) {
            smsc_log_debug(log_,"journal has no records");
        }
        return;
    }

    try {

        if (log_ && log_->isDebugEnabled()) {
            std::ostringstream os;
            for ( RecordList::const_iterator i = records.begin();
                  i != records.end();
                  ++i ) {
                os << " " << (*i)->getSerial();
            }
            smsc_log_debug(log_,"record serials(%u): %s", unsigned(records.size()),
                           os.str().c_str());
        }

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
            std::sort( records.begin(), records.end(), smsc::util::PtrLess() );
            RecordList::const_iterator i = records.begin();
            for ( RecordList::const_iterator j = i+1;
                  j != records.end(); ++j ) {
                if ( (*i)->getSerial() + 1 != (*j)->getSerial() ) {
                    if ( (*i)->getSerial() == serialShift-1 &&
                         (*j)->getSerial() == serialShift+1 ) {
                        // ok
                    } else {
                        throw smsc::util::Exception("journal records are not sequential: %u %u",
                                                    unsigned((*i)->getSerial()-serialShift),
                                                    unsigned((*j)->getSerial()-serialShift));
                    }
                }
                ++i;
            }
            // shifting back
            for ( RecordList::const_iterator ir = records.begin();
                  ir != records.end();
                  ++ir ) {
                (*ir)->setSerial( (*ir)->getSerial() - serialShift );
            }
        }

        if (log_ && log_->isDebugEnabled()) {
            std::ostringstream os;
            for ( RecordList::const_iterator i = records.begin();
                  i != records.end();
                  ++i ) {
                os << " " << (*i)->getSerial();
            }
            smsc_log_debug(log_,"after sorting: %s", os.str().c_str());
        }

        // all records are sequential
        store_.prepareForApplication( records );

        try {

            // applying new states
            ::Applier applier(store_,true);
            recordSerial_ = records.back()->getSerial();
            journalFile_.Seek( records.back()->getEndsAt() );
            std::for_each( records.begin(), records.end(), applier );
            applier.setState( records.back() );

        } catch ( std::exception& e ) {
            
            // FIXME: copying the old file
            /*
            journalFile_.Rename( (fn + ".old").c_str() );
            if ( readonly ) {
                journalFile_.ROpen(fn.c_str());
            } else {
                journalFile_.RWOpen(fn.c_str());
            }
             */
            if (log_) {
                smsc_log_info(log_,"new state failed: %s", e.what());
            }

            // trying with old
            ::Applier applier(store_,false);
            recordSerial_ = 0;
            std::for_each( records.rbegin(), records.rend(), applier );
            applier.setState( records.front() );
        }

    } catch ( ... ) {
        std::for_each( records.begin(), records.end(), smsc::util::PtrDestroy() );
        throw;
    }
    std::for_each( records.begin(), records.end(), smsc::util::PtrDestroy() );
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
    // NOTE: we don't modify recordSerial_ until it has been written to disk
    const bool needHeader = ( recordSerial_ == 0 );
    uint32_t serial = recordSerial_ + 1;
    if ( serial == 0 ) { ++serial; }
    const bool lastRecord = 
        ( ( serial % maxRecords_ ) == 0 ) ||
        ( journalFile_.Pos() > 10000000 );
    // record.setSerial( serial );

    const uint32_t jnlSize = uint32_t(constantRecordSize() + record.savedDataSize());
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
    io::EndianConverter::set32(ptr,jnlSize);
    ptr = mempcpy(ptr+4,&csum,8);
    io::EndianConverter::set32(ptr,serial);
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
        // NOTE: we need truncate as the last entry may be rewritten
        journalFile_.Truncate(journalFile_.Pos());
        journalFile_.Seek( store_.journalHeaderSize() );
    }
    if ( needHeader ) {
        journalFile_.Truncate(journalFile_.Pos());
    }
    recordSerial_ = serial;
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
    const uint32_t jnlSize = io::EndianConverter::get32(ptr);
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
    jr->setControlSum( io::EndianConverter::get64(ptr) );
    jr->setSerial( io::EndianConverter::get32(ptr+8) );
    jr->load(ptr+12,jnlSize-constantRecordSize());
    iptr = endptr;
    return jr.release();
}

} // namespace storage
} // namespace util
} // namespace scag2
