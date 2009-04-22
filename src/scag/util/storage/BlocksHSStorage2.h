#ifndef _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE2_H
#define _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE2_H

#include "util/int.h"
#include "Serializer.h"
#include "DataFileManager.h"
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

    class CreationTask;

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
    inline size_t journalHeaderSize() const {
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

    BlocksHSStorage2( DataFileManager& manager, smsc::logger::Logger* logger = 0 );

    ~BlocksHSStorage2();

    int open( const std::string& dbname, const std::string& dbpath )
    {
        if ( inited_ ) return ALREADY_INITED;
        if ( dbname.empty() ) return DBNAME_INVALID;
        dbname_ = dbname;
        dbpath_ = dbpath;
        int ret = doOpen();
        if (!ret) { inited_ = true; }
        return ret;
    }

    int create( const std::string& dbname,
                const std::string& dbpath,
                size_t fileSize,
                size_t blockSize,
                uint32_t version = 0x80000002 )
    {
        if ( inited_ ) return ALREADY_INITED;
        if ( dbname.empty() ) return DBNAME_INVALID;
        version_ = version;
        dbname_ = dbname;
        dbpath_ = dbpath;
        packer_ = HSPacker(blockSize,0,log_);
        fileSize_ = fileSize;
        fileSizeBytes_ = fileSize_ * blockSize;
        int ret = doCreate();
        if (!ret) { inited_ = true; }
        return ret;
    }


    inline void packBuffer( buffer_type& buf, buffer_type* hdr )
    {
        packer_.packBuffer(buf,hdr);
    }
    inline void unpackBuffer( buffer_type& buf, buffer_type* hdr )
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
                       const KeyLogger* kl = 0 );


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
               std::vector< offset_type >* affected );


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
                        buffer_type* buffer = 0 );


    /// the method returns free blocks back to free chain.
    /// but it does not write anything to the disk.
    /// instead it fills buffer (and posAndSize) if supplied, with
    /// new data that should be written.
    /// NOTE: affected blocks are not modified!
    bool pushFreeBlocks( size_t freeStart,
                         const std::vector< offset_type >& affectedBlocks,
                         buffer_type* buffer = 0 );

    int doOpen();

    int doCreate();

    // opening the data files according to the state
    // and (possibly) applying transaction stored in buffer.
    int openDataFiles( const StorageState& state, const buffer_type* buffer );

    inline size_t minTransactionSize() const
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


    bool readJournal( buffer_type& oldbuf,
                      buffer_type& newbuf,
                      StorageState& oldState,
                      StorageState& newState );


    bool writeJournal( const buffer_type& oldbuf,
                       const buffer_type& newbuf,
                       const StorageState& oldhead );

    
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
                      size_t initialPos = 0 );


    bool readFreeBlocks( size_t needBlocks, bool allowNewFile = true );

    /// check if we have enough free blocks
    void checkFreeCount( size_t freeCount );

    /// attaches a new file and a list of free blocks to a free chain
    /// NOTE: the last free block in the chain should have nextBlock() == notUsed().
    bool attachNewFile();

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
    bool                  inited_;

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

    DataFileManager&                   manager_;
    std::auto_ptr<CreationTask>        creationTask_;
    smsc::core::synchronization::Mutex creationMutex_;
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
