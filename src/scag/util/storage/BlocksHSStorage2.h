#ifndef _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE2_H
#define _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE2_H

#include "util/int.h"
#include "Serializer.h"
#include "HSPacker.h"
#include "scag/util/Drndm.h"
#include <cstring>
#include <vector>
#include <deque>
#include <functional>
#include <algorithm>
#include <sstream>
#include <iterator>

namespace scag2 {
namespace util {
namespace storage {

class BlocksHSStorage2
{
public:
    typedef HSPacker::buffer_type  buffer_type;
    typedef HSPacker::offset_type  offset_type;
    typedef HSPacker::index_type   index_type;

private:
    /*
    struct TransHeader
    {
        offset_type ffb;
        uint32_t    files_count;  // how many files
        uint32_t    free_count;   // how many free blocks in the chain
    };
     */

    // to keep state during transaction and for serialization
    struct StorageState
    {
        StorageState() {}

        StorageState( const BlocksHSStorage2& bhs ) :
        fileCount(bhs.files_.size()),
        freeCount(bhs.freeCount_),
        ffb(bhs.freeChain_.empty() ? bhs.packer_.invalidIndex() : bhs.freeChain_.front()) {}

        static size_t dataSize() { return 12; }

        unsigned char* saveData( unsigned char* ptr ) const
        {
            uint32_t val = htonl(fileCount);
            ptr = mempcpy(ptr,&val,4);
            val = htonl(freeCount);
            ptr = mempcpy(ptr,&val,4);
            val = htonl(ffb);
            ptr = mempcpy(ptr,&val,4);
            return ptr;
        }

        const unsigned char* loadData( const unsigned char* ptr )
        {
            uint32_t val;
            ptr = memscpy(&val,ptr,4);
            fileCount = ntohl(val);
            ptr = memscpy(&val,ptr,4);
            freeCount = ntohl(val);
            ptr = memscpy(&val,ptr,4);
            ffb = ntohl(val);
            return ptr;
        }

        uint32_t fileCount;
        uint32_t freeCount;
        uint32_t ffb;
    };


    // version + blockSize + fileSize
    size_t journalHeaderSize() const {
        return 12;
    }

    typedef std::deque< unsigned > FreeChainType;

public:
    enum {
            ALREADY_INITED = -1,
            DBNAME_INVALID = -2,
            JOURNAL_FILE_OPEN_FAILED = -16,
            DATA_FILES_OPEN_FAILED = -17,
            JOURNAL_FILE_ALREADY_EXISTS = -18,
            JOURNAL_FILE_CREATION_FAILED = -19
    };



    struct KeyLogger
    {
    public:
        virtual const char* toString() const = 0;
        virtual ~KeyLogger() {}
    };

    template < class Key > class KeyLoggerT : public KeyLogger
    {
    public:
        KeyLoggerT( const Key& key ) : key_(key) {}
        virtual const char* toString() const {
            return key_.toString().c_str();
        }
    private:
        const Key& key_;
    };

public:    
    inline size_t blockSize() const { return packer_.blockSize(); }
    inline index_type invalidIndex() const { return packer_.invalidIndex(); }

    inline uint32_t version() const { return version_; }
    inline size_t headerSize() const { return idxSize() + navSize(); }

    BlocksHSStorage2( smsc::logger::Logger* logger = 0 ) :
    packer_(1024,0,logger),
    fileSize_(0),
    fileSizeBytes_(0),
    log_(logger),
    oldBuf_(0),
    newBuf_(0),
    freeCount_(0),
    journalWrites_(0),
    maxJournalWrites_(100)
    {
        rnd_.setSeed(time(0));
    }


    ~BlocksHSStorage2() {
        delete oldBuf_;
        delete newBuf_;
    }


    int open( const std::string& dbname, const std::string& dbpath )
    {
        if ( ! dbname_.empty() ) return ALREADY_INITED;
        if ( dbname.empty() ) return DBNAME_INVALID;
        dbname_ = dbname;
        dbpath_ = dbpath;
        return doOpen();
    }


    int create( const std::string& dbname,
                const std::string& dbpath,
                size_t fileSize,
                size_t blockSize,
                uint32_t version = 0x80000002 )
    {
        if ( ! dbname_.empty() ) return ALREADY_INITED;
        if ( dbname.empty() ) return DBNAME_INVALID;
        version_ = version;
        dbname_ = dbname;
        dbpath_ = dbpath;
        packer_ = HSPacker(blockSize,0,log_);
        fileSize_ = fileSize;
        fileSizeBytes_ = fileSize_ * blockSize;
        return doCreate();
    }


    void packBuffer( buffer_type& buf, buffer_type* hdr )
    {
        packer_.packBuffer(buf,hdr);
    }

    void unpackBuffer( buffer_type& buf, buffer_type* hdr )
    {
        packer_.unpackBuffer(buf,hdr);
    }


    /// Modifies storage:
    ///
    /// the data with key 'key' (for logging only),
    /// which was stored starting at index 'oldIndex',
    /// having old datablock content in 'oldBuf'
    ///    is being replaced with
    /// a new content in 'newBuf'.
    ///
    /// Typical cases (in order of checking):
    /// 1. newBuf = 0 or newBuf->size() <= idxSize()+navSize() -- data should be removed;
    ///    precondition: oldIndex != invalidIndex()
    ///    if oldBuf == 0, it is read from storage.
    /// 2. oldIndex == invalidIndex() -- data should be appended (created).
    ///    oldBuf ignored.
    /// 3. otherwise, data should be updated.
    ///    precondition: oldBuf != 0 and oldBuf->size() > idxSize()+navSize().
    ///
    /// Return invalidIndex() on failure, the index of new data on success.
    /// NOTE: that in case of removal, it always returns invalidIndex().
    index_type change( index_type   oldIndex,
                       buffer_type* oldBuf,
                       buffer_type* newBuf,
                       const KeyLogger* kl = 0 )
    {
        size_t needBlocks;
        size_t hasBlocks;
        size_t newDataSize = 0;
        size_t oldDataSize = 0;
        std::vector< offset_type > affectedBlocks;  // offsets
        const offset_type oldPos = idx2pos(oldIndex);
        StorageState oldState(*this);

        bool ok = false;
        do {

            if (!newBuf || newBuf->size() <= idxSize()+navSize() ) {

                // no new data, removal

                if (!newBuf) {
                    if (!newBuf_) newBuf_ = new buffer_type;
                    newBuf = newBuf_;
                }
                newBuf->clear();
                needBlocks = 0;
                newDataSize = 0;

                if (oldIndex == invalidIndex()) break; // exit

                if (!oldBuf) {

                    // old buf is not passed, take it from disk
                    if (!oldBuf_) oldBuf_ = new buffer_type;
                    oldBuf = oldBuf_;
                    oldBuf->clear();
                    if ( !read(oldPos,*oldBuf,&affectedBlocks) ) {
                        // read all blocks at oldIndex failed
                        break;
                    }

                } else {

                    // we already have oldBuf, find out what blocks are affected
                    if ( !getAffectedBlocks(*oldBuf,affectedBlocks) ) {
                        // buffer parsing failed
                        // FIXME: should we try to reread from disk ?
                        break;
                    }

                }

                if ( oldBuf->size() <= idxSize() ) { 
                    // invalid old buf
                    break;
                }
                oldDataSize = oldBuf->size() - idxSize();
                hasBlocks = packer_.countBlocks(oldDataSize);
                assert(hasBlocks == affectedBlocks.size());

            } else {
            
                // newBuf passed
                newDataSize = newBuf->size() - idxSize();
                needBlocks = packer_.countBlocks(newDataSize);

                if (oldIndex == invalidIndex()) {
                    // no old data
                    if (!oldBuf) {
                        if (!oldBuf_) oldBuf_ = new buffer_type;
                        oldBuf = oldBuf_;
                    }
                    oldBuf->clear();
                    oldDataSize = 0;

                } else {

                    if ( ! getAffectedBlocks(*oldBuf,affectedBlocks) ) {
                        // buffer parsing failed
                        // FIXME: should we try to reread from disk?
                        break;
                    }
                    if ( oldBuf->size() <= idxSize() ) {
                        // invalid buffer
                        break; 
                    }
                    oldDataSize = oldBuf->size() - idxSize();
                }

                hasBlocks = packer_.countBlocks(oldDataSize);
                assert(hasBlocks == affectedBlocks.size());
                if ( hasBlocks < needBlocks ) {
                    // more free blocks are needed
                    // NOTE: side-effect: transHeader_ and free chain are changed if true.
                    if ( ! popFreeBlocks(needBlocks-hasBlocks,affectedBlocks,oldBuf) ) {
                        // free blocks failure
                        // FIXME: the method above should restore the previous state of the free chain and header
                        break;
                    }
                }
            }

            // at this point we have all information:
            // hasBlocks, needBlocks, affectedBlocks.
            // Both oldBuf and newBuf are filled with data, however newBuf is
            // not yet merged with posAndSize

            if ( newBuf->size() > idxSize() ) {
                packer_.makeHeaders(headers_,affectedBlocks,newDataSize);
                packer_.mergeHeaders(*newBuf, headers_);
            }

            if ( hasBlocks > needBlocks ) {
                // free blocks should be chained also
                // NOTE: side-effect: transHeader_ and free chain are changed if true.
                if ( !pushFreeBlocks(needBlocks,affectedBlocks,newBuf) ) {
                    // FIXME: the method above should restore the previous state of storage
                    break;
                }
            }

            // writing journal
            if ( !writeJournal(*oldBuf, *newBuf, oldState) ) {
                // cannot save transaction
                rollbackFreeChain(hasBlocks,needBlocks,affectedBlocks);
                break;
            }

            // calculating position-and-size of blocks to write
            std::vector< offset_type > posAndSize;
            packer_.offsetsToPosAndSize( affectedBlocks, newDataSize, posAndSize );

            // saving blocks
            if ( !writeBlocks(*newBuf,&posAndSize) ) {
                // failure
                writeBlocks(*oldBuf);
                rollbackFreeChain(hasBlocks,needBlocks,affectedBlocks);
                break;
            }

            ok = true;

        } while ( false );

        if ( ! ok ) {
            // FIXME: rollback profile, etc.
            oldIndex = invalidIndex();
        } else if ( needBlocks > 0 ) {
            oldIndex = pos2idx(affectedBlocks[0]);
        }
        if ( oldDataSize ) { oldBuf->resize(oldDataSize+idxSize()); }
        if ( newDataSize ) { newBuf->resize(newDataSize+idxSize()); }
        return oldIndex;
    }


    /// Read data, starting from index 'index' to the buffer buf (as is, i.e. w/o unpacking).
    /// Return true, if the read was successful.
    bool read(index_type index,buffer_type& buf) {
        buf.clear();
        return read(idx2pos(index),buf,0);
    }


private:
    inline size_t idxSize() const { return packer_.idxSize(); }
    inline size_t navSize() const { return packer_.navSize(); }
    inline index_type pos2idx( offset_type o ) const { return packer_.pos2idx(o); }
    inline offset_type idx2pos( index_type i ) const { return packer_.idx2pos(i); }

    /// reading used blocks at offset.
    bool read( offset_type  offset,
               buffer_type& data,
               std::vector< offset_type >* affected )
    {
        if ( packer_.isNotUsed(offset) ) return false;
        const size_t oldSize = data.size();
        const offset_type refBlock = offset;
        /*
        data.reserve(oldSize+headerSize());
        {
            Serializer ser(data);
            ser.setwpos(oldSize);
            ser << offset;
        }
         */

        bool rv = false;
        bool first = true;
        unsigned char headbuf[BlockNavigation::NAV_SIZE];
        unsigned char* ptr = headbuf;
        size_t dataSize = navSize();

        do {

            File* f = getFile(offset);
            if (!f) {
                break;
            }
            const off_t localOffset = getOffset(offset);
            f->Seek(localOffset,SEEK_SET);
            size_t toRead = std::min(dataSize,blockSize());
            f->Read(ptr,toRead);

            BlockNavigation bn;
            bn.loadPtr(ptr);
            ptr += toRead;
            
            if (first) {

                // first block
                if (!bn.isHead()) { break; }
                if (bn.dataSize() <= navSize()) {
                    break;
                }
                if ( affected ) {
                    affected->clear();
                    affected->reserve(packer_.countBlocks(bn.dataSize()));
                }
                data.resize(oldSize+idxSize()+bn.dataSize());
                BlockNavigation::saveIdx(&data[oldSize],refBlock);
                ptr = std::copy( headbuf, headbuf+navSize(), &data[oldSize+idxSize()] );

                dataSize = bn.dataSize()-toRead;
                // reading the rest of the block
                toRead = std::min(dataSize,blockSize()-navSize());
                f->Read(ptr,toRead);
                ptr += toRead;
                first = false;

            } else {

                // not a first block in chain
                if (bn.isFree() || bn.isHead()) {
                    break;
                }
                if (bn.refBlock() != refBlock) {
                    break;
                }

            }
            dataSize -= toRead;
            if ( affected ) affected->push_back(offset);

            if ( dataSize == 0 ) {
                if ( packer_.isNotUsed(bn.nextBlock()) ) {
                    // last block
                    rv = true; 
                }
                break;
            } else {
                if ( dataSize < navSize() ) {
                    // datasize is broken
                    break;
                }
                offset = bn.nextBlock();
                if ( packer_.isNotUsed(offset) ) {
                    // not enough blocks
                    break;
                }
            }

        } while (true);
        if (!rv) {
            if ( data.size() > oldSize ) {
                // strip the rest of buffer
                data.resize(ptr - &data[0]);
            }
        }
        return rv;
    }


    bool getAffectedBlocks( const buffer_type& buf,
                            std::vector<offset_type>& offsets )
    {
        std::vector< offset_type > blocks;
        blocks.reserve( countBlocksPerBuf(buf.size())*2+2 );
        packer_.extractBlocks(buf,blocks,packer_.notUsed());
        packer_.extractOffsets(blocks,offsets);
        return true;
    }


    /// pop a 'needBlocks' blocks from free chain,
    /// storing them in affectedBlocks and in buffer.
    bool popFreeBlocks( size_t       needBlocks,
                        std::vector< offset_type >& affectedBlocks,
                        buffer_type* buffer = 0 )
    {
        // const size_t oldAffSize = affectedBlocks.size();
        if ( freeChain_.size() <= needBlocks ) {
            if ( !readFreeBlocks(needBlocks-freeChain_.size()+1) ) {
                return false; 
            }
        }
        
        // NOTE: we transform one block more to have it chained at the end
        std::vector<offset_type> temp(needBlocks+1);
        std::transform( freeChain_.begin(),
                        freeChain_.begin()+needBlocks+1,
                        temp.begin(),
                        std::bind2nd(std::multiplies<offset_type>(),packer_.blockSize()) );
        if (log_ && log_->isDebugEnabled()) {
            std::ostringstream os;
            os.setf( std::ios::hex, std::ios::basefield );
            std::copy( temp.begin(), temp.begin()+needBlocks,
                       std::ostream_iterator< offset_type >(os," ") );
            smsc_log_debug(log_,"pop %u free blocks: %s", unsigned(needBlocks), os.str().c_str() );
        }
        affectedBlocks.insert( affectedBlocks.end(), temp.begin(), temp.begin()+needBlocks );

        // makeup buffer of free block headers
        if ( buffer ) {

            size_t oldBufSize = buffer->size();
            buffer->resize(oldBufSize+idxSize()+navSize()*needBlocks);
            std::vector< offset_type >::iterator iter = temp.begin();
            unsigned char* ptr = &(*buffer)[oldBufSize];
            BlockNavigation::saveIdx(ptr,*iter);
            ptr += idxSize();
            BlockNavigation bn;
            size_t freeCount = freeCount_;
            while ( freeCount <= needBlocks ) { freeCount += fileSize_; }
            for ( size_t i = 0; i < needBlocks; ++i ) {
                bn.setFreeCells(--freeCount);
                bn.setNextBlock(*++iter);
                bn.savePtr(ptr);
                ptr += navSize();
            }

        }
        freeChain_.erase( freeChain_.begin(), freeChain_.begin() + needBlocks );
        freeCount_ -= needBlocks;
        return true;
    }


    /// the method returns free blocks back to free chain.
    /// but it does not write anything to the disk.
    /// instead it fills buffer (and posAndSize) if supplied, with
    /// new data that should be written.
    /// NOTE: affected blocks are not modified!
    bool pushFreeBlocks( size_t freeStart,
                         const std::vector< offset_type >& affectedBlocks,
                         buffer_type* buffer = 0 )
    {
        if ( freeStart >= affectedBlocks.size() ) {
            if (log_) { 
                smsc_log_error(log_,"logic error: too few affected blocks in pushFree");
            }
            abort();
        }
        if ( freeChain_.size() == 0 ) {
            if ( ! readFreeBlocks(1) ) {
                return false; 
            }
        }

        const size_t returnedBlocks = affectedBlocks.size() - freeStart;
        std::vector< unsigned > temp(returnedBlocks);
        std::transform( affectedBlocks.begin()+freeStart,
                        affectedBlocks.end(),
                        temp.begin(),
                        std::bind2nd(std::divides< offset_type >(),packer_.blockSize()) );
        if ( log_ && log_->isDebugEnabled() ) {
            std::ostringstream os;
            os.setf( std::ios::hex, std::ios::basefield );
            std::copy( affectedBlocks.begin()+freeStart,
                       affectedBlocks.end(),
                       std::ostream_iterator< offset_type >(os," ") );
            smsc_log_debug(log_, "push %u free blocks: %s", unsigned(returnedBlocks), os.str().c_str() );
        }
        
        // makeup buffer of free blocks
        if ( buffer ) {
            size_t oldBufSize = buffer->size();
            buffer->resize( oldBufSize + idxSize() + navSize()*returnedBlocks );
            std::vector< offset_type >::const_iterator iter = affectedBlocks.begin() + freeStart;
            unsigned char* ptr = &(*buffer)[oldBufSize];
            BlockNavigation::saveIdx(ptr,*iter);
            ptr += idxSize();
            BlockNavigation bn;
            size_t freeCount = freeCount_;
            while ( freeCount <= returnedBlocks ) { freeCount += fileSize_; }
            for ( size_t i = 0; i < returnedBlocks; ++i ) {
                bn.setFreeCells(--freeCount);
                if ( ++iter == affectedBlocks.end() ) {
                    bn.setNextBlock(idx2pos(freeChain_.front()));
                } else {
                    bn.setNextBlock(*++iter);
                }
                bn.savePtr(ptr);
                ptr += navSize();
            }
        }
        freeChain_.insert( freeChain_.begin(), temp.begin(), temp.end() );
        freeCount_ += returnedBlocks;
        return true;
    }


    int doOpen()
    {
        // opening journal file
        const std::string fn = dbpath_ + "/" + dbname_ + ".jnl";
        try {
            journalFile_.RWOpen( fn.c_str() );
            journalFile_.SetUnbuffered();
            // reading the head
            journal_.resize(journalHeaderSize());
            if ( journalHeaderSize() != journalFile_.Read(&journal_[0],journalHeaderSize()) ) {
                if (log_) {
                    smsc_log_error(log_,"file %s has too small journal file", fn.c_str());
                }
                return JOURNAL_FILE_OPEN_FAILED;
            }
            // deserialization
            Deserializer dsr(journal_);
            uint32_t val;
            dsr >> val;
            version_ = val;
            dsr >> val;
            packer_ = HSPacker(val,0,log_);
            dsr >> val;
            fileSize_ = val;
            fileSizeBytes_ = fileSize_ * packer_.blockSize();
        } catch ( std::exception& e ) {
            if (log_) {
                if ( File::Exists(fn.c_str()) ) {
                    smsc_log_error(log_,"file %s exists, but cannot be read: %s", fn.c_str(), e.what());
                } else {
                    smsc_log_error(log_,"file %s does not exist: %s", fn.c_str(), e.what());
                }
            }
            return JOURNAL_FILE_OPEN_FAILED;
        }

        if (log_) {
            smsc_log_debug(log_,"journal header has been read: version=%x blockSize=%u fileSize=%u",
                           unsigned(version_), unsigned(packer_.blockSize()), unsigned(fileSize_));
        }

        // reading transactions
        size_t transRead = 0;
        StorageState newState;
        StorageState oldState;
        try {
            if ( !oldBuf_ ) oldBuf_ = new buffer_type;
            if ( !newBuf_ ) newBuf_ = new buffer_type;
            do {
                if ( !readJournal( *oldBuf_, *newBuf_, oldState, newState ) ) {
                    break;
                }
                ++transRead;
            } while ( true );
        } catch ( std::exception& e ) {
            if (transRead == 0 && log_) {
                smsc_log_warn(log_,"readJournal: %s", e.what());
            }
        }
        if ( transRead == 0 ) {
            // no transaction has been read
            return JOURNAL_FILE_OPEN_FAILED;
        }
        
        // else the last valid transaction is in oldstate, newstate, oldbuf, newbuf.
        // trying to open data files according to newstate.
        const char* transType = "none";
        int ret;
        do {
            try {
                ret = openDataFiles( newState, newBuf_ );
                if ( ret != 0 ) {
                    throw smsc::util::Exception("cannot open datafiles");
                }
                transType = "new";
            } catch ( std::exception& e ) {
                if (log_) {
                    smsc_log_warn(log_,"new state failure: %s", e.what());
                }
                ret = DATA_FILES_OPEN_FAILED;
            }
        } while ( false );

        // trying the same with old transaction
        do {
            if ( ret == 0 ) {break;}
            try {
                ret = openDataFiles( oldState, oldBuf_ );
                if ( ret != 0 ) {
                    throw smsc::util::Exception("cannot open datafiles");
                }
                transType = "old";
            } catch ( std::exception& e ) {
                if (log_) {
                    smsc_log_warn(log_,"old state failure: %s", e.what());
                }
                ret = DATA_FILES_OPEN_FAILED;
            }
        } while ( false );

        if ( ret == 0 ) {
            journalFile_.Seek( journalHeaderSize() );
            if (log_) {
                smsc_log_info(log_,"storage has been opened via %s trans: version=%x blockSize=%u/%x fileSize=%u/%x fszBytes=%llx fileCount=%u freeCount=%u ffb=%llx",
                              transType,
                              unsigned(version_), unsigned(packer_.blockSize()), unsigned(packer_.blockSize()),
                              unsigned(fileSize_), unsigned(fileSize_), fileSizeBytes_,
                              unsigned(files_.size()), unsigned(freeCount_), idx2pos(freeChain_.front()));
            }
        }
        return ret;
    }


    int doCreate()
    {
        const std::string fn = dbpath_ + "/" + dbname_ + ".jnl";
        if ( File::Exists(fn.c_str()) ) {
            return JOURNAL_FILE_ALREADY_EXISTS;
        }
        try {
            journalFile_.RWOpen( fn.c_str() );
            journalFile_.SetUnbuffered();
            journal_.clear();
            journal_.reserve(journalHeaderSize());
            Serializer ser(journal_);
            ser << uint32_t(version_);
            ser << uint32_t(packer_.blockSize());
            ser << uint32_t(fileSize_);
            assert( journal_.size() == journalHeaderSize());
            journalFile_.Write(&journal_[0],journalHeaderSize());

            // making a data file
            if ( ! attachNewFile() ) {
                throw smsc::util::Exception("cannot create the first datafile");
            }

        } catch ( std::exception& e ) {
            if (log_) {
                smsc_log_error(log_,"cannot create journal file %s: %s", fn.c_str(), e.what());
            }
            return JOURNAL_FILE_CREATION_FAILED;
        }

        if (log_) {
            smsc_log_info(log_,"storage has been created: version=%x blockSize=%u/%x fileSize=%u/%x fszBytes=%llx fileCount=%u freeCount=%u ffb=%llx",
                          unsigned(version_), unsigned(packer_.blockSize()), unsigned(packer_.blockSize()),
                          unsigned(fileSize_), unsigned(fileSize_), fileSizeBytes_,
                          unsigned(files_.size()), unsigned(freeCount_), idx2pos(freeChain_.front()));
        }
        return 0;
    }


    // opening the data files according to the state
    // and (possibly) applying transaction stored in buffer.
    int openDataFiles( const StorageState& state, const buffer_type* buffer )
    {
        for ( std::vector< File* >::iterator i = files_.begin(); 
              i != files_.end();
              ++i ) {
            (*i)->Close();
        }
        files_.clear();

        for ( size_t i = 0; i < state.fileCount; ++i ) {
            std::string fn = makeFileName( i );
            if ( ! File::Exists(fn.c_str()) ) {
                throw smsc::util::Exception( "file %s does not exist", fn.c_str());
            }
            std::auto_ptr< File > file(new File);
            file->RWOpen(fn.c_str());
            file->SetUnbuffered();
            files_.push_back(file.release());
        }
        
        if ( buffer ) {
            if ( !writeBlocks(*buffer) ) {
                throw smsc::util::Exception( "cannot apply transaction after opening datafiles" );
            }
        }

        // loading free chain
        freeChain_.clear();
        freeCount_ = state.freeCount;
        freeChain_.push_back( state.ffb );
        if ( ! readFreeBlocks(1) ) {
            throw smsc::util::Exception( "cannot read free blocks starting at %llx",
                                         idx2pos(state.ffb) );
        }
        return 0;
    }


    size_t minTransactionSize() const
    {
        static const size_t constantBufSize =
            4*2 +                             // length
            8*2 +                             // csum*2
            // 8*2 +                          // tstamp*2
            StorageState::dataSize()*2 +      // old+new
            4 +                               // oldbufsize
            4;                                // newbufsize
        return constantBufSize;
    }


    size_t maxTransactionSize() const
    {
        return 1100000;
    }

    bool readJournal( buffer_type& oldbuf,
                      buffer_type& newbuf,
                      StorageState& oldState,
                      StorageState& newState )
    {
        journal_.clear();
        uint32_t val;
        int rv = journalFile_.Read(&val,4);
        if ( rv == 0 ) { return false; }
        if ( rv != 4 ) {
            throw smsc::util::Exception("wrong size of transLen=%d",rv);
        }
        const size_t transactionSize = ntohl(val);
        if ( transactionSize < minTransactionSize() ) {
            throw smsc::util::Exception("weird transLen=%u, must be >= minLen=%u",
                                        unsigned(transactionSize),
                                        unsigned(minTransactionSize()) );
        }
        const uint64_t fsz( journalFile_.Size() );
        if ( transactionSize >= fsz ) {
            throw smsc::util::Exception("weird transLen=%u, must be < %u",
                                        unsigned(transactionSize),
                                        unsigned(fsz));
        }
        journal_.resize(transactionSize);
        const unsigned char* ptr = mempcpy(&journal_[0],&val,4);
        if ( transactionSize-4 != journalFile_.Read(&journal_[4],transactionSize-4) ) {
            throw smsc::util::Exception("cannot read trans data");
        }
        // checking checksums
        uint64_t csum1, csum2;
        uint32_t ctrlval;
        ptr = memscpy( &csum1, ptr, 8);
        if ( csum1 == 0 ) {
            throw smsc::util::Exception("control sum cannot be 0");
        }
        {
            const unsigned char* endptr = &journal_[transactionSize-12];
            endptr = memscpy( &csum2, endptr,8);
            memscpy( &ctrlval, endptr, 4);
        }
        if ( csum1 != csum2 ) {
            throw smsc::util::Exception("control sums do not match %llx != %llx", csum1, csum2 );
        }
        if ( val != ctrlval ) {
            throw smsc::util::Exception("lengths do not match %x != %x", unsigned(val), unsigned(ctrlval) );
        }
        // everything is ok, reading states and buffers
        ptr = oldState.loadData(ptr);
        ptr = newState.loadData(ptr);
        uint32_t szval;
        ptr = memscpy(&szval,ptr,4);
        szval = ntohl(szval);
        oldbuf.resize(szval);
        if (szval > 0 ) ptr = memscpy(&oldbuf[0],ptr,szval);
        ptr = memscpy(&szval,ptr,4);
        szval = ntohl(szval);
        newbuf.resize(szval);
        if (szval > 0 ) ptr = memscpy(&newbuf[0],ptr,szval);
        return true;
    }


    bool writeJournal( const buffer_type& oldbuf,
                       const buffer_type& newbuf,
                       const StorageState& oldhead )
    {
        size_t constantBufSize = minTransactionSize();
        const uint32_t jnlSize = constantBufSize + oldbuf.size() + newbuf.size();
        journal_.resize( jnlSize );
        unsigned char* ptr = &journal_[0];
        const uint32_t jnlSizeNet = htonl(jnlSize);

        StorageState newhead(*this);

        // composing buffer
        // control sum
        uint64_t csum;
        do {
            csum = rnd_.getNextNumber();
        } while ( csum == 0 );
        // timestamp -- not written
        // cvt.set( int64_t(util::currentTimeMillis()) );
        // const uint64_t tstamp = cvt.cvt.quads[0];
        ptr = mempcpy(ptr,&jnlSizeNet,4);
        ptr = mempcpy(ptr,&csum,8);
        // ptr = mempcpy(ptr,&tstamp,8);
        ptr = oldhead.saveData(ptr);
        ptr = newhead.saveData(ptr);
        uint32_t szval = htonl(uint32_t(oldbuf.size()));
        ptr = mempcpy(ptr,&szval,4);
        if (oldbuf.size()>0) ptr = mempcpy(ptr,&oldbuf[0],oldbuf.size());
        szval = htonl(uint32_t(newbuf.size()));
        ptr = mempcpy(ptr,&szval,4);
        if (newbuf.size()>0) ptr = mempcpy(ptr,&newbuf[0],newbuf.size());
        ptr = mempcpy(ptr,&csum,8);
        // ptr = mempcpy(ptr,&tstamp,8);
        ptr = mempcpy(ptr,&jnlSizeNet,4);
        assert( ptr == &journal_[jnlSize] );

        // write buffer to a journal file
        if ( ++journalWrites_ > maxJournalWrites_ ) {
            journalFile_.Seek(journalHeaderSize());
            journalWrites_ = 1;
        }
        if ( log_ ) {
            smsc_log_debug(log_,"writing journal #%u of sz=%u csum=%llx",
                           unsigned(journalWrites_),
                           unsigned(jnlSize),
                           csum );
        }
        journalFile_.Write( &journal_[0], jnlSize );
        return true;
    }

    
    /// method invokes pushFree or popFree, depending on what is bigger hasBlocks or needBlocks.
    /// NOTE: this method should not fail as it just return blocks to free chain
    /// w/o access to disk.
    void rollbackFreeChain( size_t hasBlocks,
                            size_t needBlocks,
                            std::vector< offset_type >& affectedBlocks )
    {
        if ( hasBlocks > needBlocks ) {
            // free blocks have been returned to storage,
            // now we have to get them back
            affectedBlocks.resize(needBlocks);
            assert(popFreeBlocks(hasBlocks-needBlocks,affectedBlocks));
        } else if ( needBlocks > hasBlocks ) {
            // free blocks have been borrowed from storage,
            // now we have to put them back
            assert(pushFreeBlocks(hasBlocks,affectedBlocks));
        }
    }


    /// writing blocks to disk.
    /// posAndSize may be supplied to avoid redundant buffer parsing.
    bool writeBlocks( const buffer_type& buffer,
                      const std::vector< offset_type >* posAndSize = 0,
                      size_t initialPos = 0 )
    {
        if ( buffer.size() < initialPos + idxSize() + navSize() ) { return true; }
        if ( ! posAndSize ) {
            posAndSize = &posAndSize_;
            packer_.extractBlocks( buffer, posAndSize_, packer_.notUsed(), initialPos );
        }
        // writing blocks
        assert( posAndSize->size() % 2 == 0 );
        const unsigned char* ptr = &buffer[initialPos];
        for ( std::vector< offset_type >::const_iterator iter = posAndSize->begin();
              iter != posAndSize->end();
              ++iter ) {
            const offset_type offset = *iter;
            const size_t sz = *++iter;
            if ( offset == packer_.notUsed() || sz == 0 ) {
                // if (log_) {
                // smsc_log_debug(log_,"skipping pos=%llx sz=%u",offset,unsigned(sz));
                // }
                ptr += idxSize();
                continue; // skip index
            }
            if (log_) {
                smsc_log_debug(log_,"writing blk=%llx sz=%u",offset,unsigned(sz));
            }
            File* f = getFile(offset);
            if (!f) { return false; }
            try {
                f->Seek( getOffset(offset) );
                f->Write(ptr,sz);
            } catch ( std::exception& e ) {
                if (log_) {
                    smsc_log_warn(log_,"problem writing blocks: %s", e.what()); 
                }
                return false;
            }
            ptr += sz;
        }
        return true;
    }


    bool readFreeBlocks( size_t needBlocks, bool allowNewFile = true )
    {
        const size_t freeChainSize = freeChain_.size();
        assert( ! freeChain_.empty() );
        offset_type lastBlock = idx2pos(freeChain_.back());
        bool rv = false;
        // hardcoded value
        size_t counter = needBlocks + 100;
        while ( true ) {

            File* f = getFile(lastBlock);
            if (!f) {
                // fail
                break;
            }

            BlockNavigation bn;
            try {
                f->Seek(getOffset(lastBlock));
                bn.load(*f);
            } catch ( std::exception& e ) {
                if (log_) {
                    smsc_log_error(log_,"cannot read block at %llx (%u/%llx): %s",
                                   lastBlock,
                                   unsigned(lastBlock / fileSizeBytes_),
                                   lastBlock % fileSizeBytes_,
                                   e.what());
                }
                break;
            }
            if ( !bn.isFree() ) {
                if (log_) {
                    smsc_log_error( log_,"block %llx (%u/%llx) should be free, but is not, file corrupted.",
                                    lastBlock,
                                    unsigned(lastBlock / fileSizeBytes_),
                                    lastBlock % fileSizeBytes_ );
                }
                break;
            }
            if ( packer_.isNotUsed(bn.nextBlock()) ) {
                // check if the storage is consistent:
                // freeCount_ must be equal to freeChain_.size()
                if ( freeCount_ != freeChain_.size() ) {
                    if (log_) {
                        smsc_log_warn( log_, "discrepancy found: freeCount=%u while freeChain.size()=%u, fixing",
                                       unsigned(freeCount_),
                                       unsigned(freeChain_.size()) );
                    }
                    freeCount_ = freeChain_.size();
                }
                // we need to attach a file
                if ( ! allowNewFile ) {
                    break;
                }
                if ( ! attachNewFile() ) {
                    break;
                }
                if ( freeChain_.size() - freeChainSize >= needBlocks ) {
                    rv = true;
                    break;
                }

                // it will increase freeCount_ and fill freeChain_ up to some level
                lastBlock = idx2pos(freeChain_.back());
                // fix counter
                counter = needBlocks - (freeChain_.size()-freeChainSize) + 2;
                continue;
            }

            lastBlock = bn.nextBlock();
            freeChain_.push_back(pos2idx(lastBlock));
            if ( --counter == 0 ) {
                rv = true;
                break;
            }

        } // while
        if ( log_ && log_->isDebugEnabled() ) {
            std::ostringstream os;
            os.setf( std::ios::hex, std::ios::basefield );
            std::transform( freeChain_.begin() + freeChainSize,
                            freeChain_.end(),
                            std::ostream_iterator< offset_type >(os," "),
                            std::bind2nd(std::multiplies<offset_type>(),packer_.blockSize()) );
            smsc_log_debug(log_,"free chain upload +sz=%u: %s",
                           unsigned(freeChain_.size()-freeChainSize),
                           os.str().c_str());
        }
        return rv;
    }


    /// attaches a new file and a list of free blocks to a free chain
    /// NOTE: the last free block in the chain should have nextBlock() == notUsed().
    bool attachNewFile()
    {
        // FIXME
        return false;
    }


    /// copy sz bytes from 'src' to 'dst' and returns the new position of dst.
    /// the function is ala linux mempcpy.
    inline static unsigned char* mempcpy(unsigned char* dst, const void* src, unsigned sz)
    {
        memcpy(dst,src,sz);
        return dst+sz;
    }

    /// copy sz bytes from 'src' to 'dst' and returns the new position of src.
    inline static const unsigned char* memscpy(void* dst, const unsigned char* src, unsigned sz )
    {
        memcpy(dst,src,sz);
        return src+sz;
    }


    std::string makeFileName( size_t idx ) const
    {
        char buf[50];
        snprintf(buf,sizeof(buf),"-%.7u",unsigned(idx));
        std::string rv;
        rv.reserve( dbpath_.size() + dbname_.size() + strlen(buf) + 2 );
        rv.append(dbpath_);
        rv.push_back('/');
        rv.append(dbname_);
        rv.append(buf);
        return rv;
    }


    inline File* getFile( offset_type pos ) {
        unsigned fn = pos / fileSizeBytes_;
        if ( fn >= files_.size() ) {
            if (log_) {
                smsc_log_error(log_,"position %llx is in fn=%u >= %u",pos,fn,files_.size());
            }
            return 0;
        }
        return files_[fn];
    }
    
    inline offset_type getOffset( offset_type pos ) const {
        return ( pos % fileSizeBytes_ );
    }
    
    inline size_t countBlocksPerBuf( size_t packedSize ) const {
        if ( packedSize <= idxSize() ) return 0;
        return packer_.countBlocks(packedSize-idxSize());
    }

private:
    uint32_t              version_;       // version
    HSPacker              packer_;
    size_t                fileSize_;      // in blocks
    offset_type           fileSizeBytes_; // in bytes
    smsc::logger::Logger* log_;

    std::string           dbpath_;
    std::string           dbname_;

    // working buffers (to prevent reallocation)
    buffer_type*          oldBuf_;     // owned
    buffer_type*          newBuf_;     // owned
    buffer_type           headers_;    // working place for headers
    buffer_type           journal_;    // journal
    std::vector< offset_type > posAndSize_;

    // the state of the storage
    std::vector< File* >  files_;
    size_t                freeCount_;  // how many free cells in storage
    std::deque<unsigned>  freeChain_;  // indices (not offsets!)

    util::Drndm           rnd_;

    size_t                journalWrites_; // counter for how many times a journal was written
    size_t                maxJournalWrites_;
    File                  journalFile_;
};

} // namespace storage
} // namespace util
} // namespace scag

namespace scag {
namespace util {
namespace storage {
using namespace scag2::util::storage;
} // namespace storage
} // namespace util
} // namespace scag

#endif /* ! _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE2_H */
