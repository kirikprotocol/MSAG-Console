#ifndef _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE2_H
#define _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE2_H

#include <cstring>
#include <vector>
#include <deque>
#include <functional>
#include <algorithm>
#include <sstream>
#include <iterator>

#include "util/int.h"
#include "scag/util/io/Serializer.h"
#include "DataFileManager.h"
#include "HSPacker.h"
#include "KeyLogger.h"
#include "JournalFile.h"

namespace scag2 {
namespace util {
namespace storage {

class BlocksHSStorage2 : public JournalStorage
{
public:
    typedef HSPacker::buffer_type  buffer_type;
    typedef HSPacker::offset_type  offset_type;
    typedef HSPacker::index_type   index_type;

private:
    class CreationTask;
    struct StorageState;
    struct Transaction;
    typedef std::deque< unsigned > FreeChainType;
    class FreeChainRescuer;

    /// scanning the whole storage returning true
    /// when the head is found
    struct Scanner {
        Scanner( BlocksHSStorage2* s = 0, FreeChainRescuer* f = 0 ) :
        s_(s), f_(f), idx_(-1) {}
        bool next();
    public:
        BlocksHSStorage2*  s_; // pointer to the storage
        FreeChainRescuer*  f_;
        index_type         idx_;
    };

public:
    /// interface for rescueing indices.
    /// Please, inherits from this and pass to recover().
    struct IndexRescuer
    {
        typedef BlocksHSStorage2::index_type  index_type;
        typedef BlocksHSStorage2::buffer_type buffer_type;
        virtual ~IndexRescuer() {}
        virtual void recoverIndex( index_type idx, buffer_type& buffer ) = 0;
    };


    class Iterator
    {
    public:
        Iterator() {}
        Iterator( BlocksHSStorage2& s ) : scanner_(&s) {}
        bool next();

        inline index_type getIndex() const { return scanner_.idx_; }
        inline buffer_type& getBuffer() { return buffer_; }

    private:
        Scanner     scanner_;
        buffer_type buffer_;
    };

public:
    enum {
            ALREADY_INITED = -1,
            DBNAME_INVALID = -2,
            JOURNAL_FILE_OPEN_FAILED = -16,
            DATA_FILES_OPEN_FAILED = -17,
            JOURNAL_FILE_ALREADY_EXISTS = -18,
            JOURNAL_FILE_CREATION_FAILED = -19,
            FREE_CHAIN_BROKEN = -20,
            JOURNAL_FILE_READ_FAILED = -21,
            TRANSACTION_WRITE_FAILED = -22
    };

public:    
    BlocksHSStorage2( DataFileManager& manager, smsc::logger::Logger* logger = 0 );

    ~BlocksHSStorage2();

    inline void setKeyLogger( const KeyLogger& kl ) { keylogger_ = &kl; }

    int open( const std::string& dbname,
              const std::string& dbpath,
              bool  readonly = false );

    int create( const std::string& dbname,
                const std::string& dbpath,
                size_t fileSize,
                size_t blockSize,
                uint32_t version = 0x80000002 );


    int recover( const std::string& dbname,
                 const std::string& dbpath,
                 IndexRescuer* indexRescuer = 0,
                 uint32_t version = 0x80000002 );

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
    ///    if oldBuf == 0, it is read from storage.
    ///    precondition: oldBuf->size() > idxSize()+navSize().
    ///
    /// Return invalidIndex() on failure, the index of new data on success.
    /// NOTE: that in case of removal, it always returns invalidIndex().
    index_type change( index_type   oldIndex,
                       buffer_type* oldBuf,
                       buffer_type* newBuf );


    /// Read data, starting from index 'index' to the buffer buf (as is, i.e. w/o unpacking).
    /// Return true, if the read was successful.
    bool read(index_type index,buffer_type& buf) {
        buf.clear();
        return read(idx2pos(index),buf,0);
    }

    inline void packBuffer( buffer_type& buf, buffer_type* hdr )
    {
        packer_.packBuffer(buf,hdr);
    }
    inline void unpackBuffer( buffer_type& buf, buffer_type* hdr )
    {
        packer_.unpackBuffer(buf,hdr);
    }

    inline size_t blockSize() const { return packer_.blockSize(); }
    inline index_type invalidIndex() const { return packer_.invalidIndex(); }

    inline uint32_t version() const { return version_; }
    inline size_t headerSize() const { return idxSize() + navSize(); }

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
    int doRecover( IndexRescuer* indexRescuer );


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
    bool writeBlocks( const void* buffer,
                      size_t bufsize,
                      const std::vector< offset_type >* posAndSize = 0,
                      size_t initialPos = 0 );


    bool readFreeBlocks( size_t needBlocks, bool allowNewFile = true );

    /// check if we have enough free blocks
    void checkFreeCount( size_t freeCount );

    /// attaches a new file and a list of free blocks to a free chain
    /// NOTE: the last free block in the chain should have nextBlock() == notUsed().
    bool attachNewFile();

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
        unsigned fn = unsigned(pos / fileSizeBytes_);
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

    // --- journal storage interface

    virtual std::string journalFileName() const {
        return dbpath_ + "/" + dbname_ + ".jnl";
    }
    
    // version + blockSize + fileSize
    virtual size_t journalHeaderSize() const { return 12; }
    virtual size_t maxJournalHeaderSize() const { return journalHeaderSize(); }

    virtual const std::string& journalRecordMark() const;
    virtual const std::string& journalRecordTrailer() const;

    virtual void saveJournalHeader( void* buffer ) const;
    virtual size_t loadJournalHeader( const void* buffer );
    virtual JournalRecord* createJournalRecord();
    virtual void prepareForApplication( const std::vector< JournalRecord* >& records );
    virtual void applyJournalData( const JournalRecord& rec, bool takeNew );
    virtual void applyJournalState( const JournalRecord& rec, bool takeNew );

    /*
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
     */

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
    // buffer_type           journal_;    // journal
    std::vector< offset_type > posAndSize_;

    // the state of the storage
    std::vector< File* >  files_;
    size_t                freeCount_;  // how many free cells in storage
    std::deque<unsigned>  freeChain_;  // indices (not offsets!)

    // util::Drndm           rnd_;

    // uint32_t              journalWrites_; // counter for how many times a journal was written
    // size_t                maxJournalWrites_;
    JournalFile              journalFile_;

    DataFileManager&                   manager_;
    std::auto_ptr<CreationTask>        creationTask_;
    smsc::core::synchronization::Mutex creationMutex_;
    const KeyLogger*                   keylogger_;

    bool                               readonly_;
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
