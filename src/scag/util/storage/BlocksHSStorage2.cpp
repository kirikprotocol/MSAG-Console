#include <algorithm>
#include <cmath>
#include "BlocksHSStorage2.h"
#include "scag/util/WatchedThreadedTask.h"
#include "DataFileManager.h"
#include "scag/util/Time.h"
#include "scag/util/PtrLess.h"
#include "scag/util/PtrDestroy.h"

namespace {

bool staticDone = false;
scag2::util::storage::BlocksHSStorage2::buffer_type transactionHeader;
scag2::util::storage::BlocksHSStorage2::buffer_type transactionEnd;
smsc::core::synchronization::Mutex staticMutex;

void makeStaticData()
{
    if ( staticDone ) return;
    MutexGuard mg(staticMutex);
    if ( staticDone ) return;
    const std::string transHead("TrAnSaCt");
    const std::string transEnd("EnDoFtRaNsAcTiOnS");
    std::copy( transHead.begin(), transHead.end(), std::back_inserter(transactionHeader) );
    std::copy( transEnd.begin(), transEnd.end(), std::back_inserter(transactionEnd) );
    staticDone = true;
}

}


namespace scag2 {
namespace util {
namespace storage {


class BlocksHSStorage2::CreationTask : public WatchedThreadedTask
{
public:
    typedef std::vector< unsigned > FreeChainType;

public:
    CreationTask( const std::string& fileName,
                  size_t             blockSize,
                  size_t             fileSize,
                  size_t             fileCount,
                  size_t             speedKbSec,
                  smsc::logger::Logger* logger = 0 ) :
    fileName_(fileName), packer_(blockSize,0/*,logger*/), fileSize_(fileSize),
    fileCount_(fileCount), speed_(speedKbSec), log_(logger) {
    }
    virtual ~CreationTask() {
    }

    virtual const char* taskName() { return "bhs.preallocation"; }

    virtual void stop() {
        MutexGuard mg(sleepMonitor_);
        sleepMonitor_.notify();
        WatchedThreadedTask::stop();
    }


    smsc::core::buffers::File* getFile() {
        return file_.release();
    }

    inline FreeChainType& freeChain() { return freeChain_; }

    inline size_t fileSize() const { return fileSize_; }


private:
    virtual int doExecute()
    {
        const std::string tempName = fileName_ + ".tmp";
        if ( checkFile(fileName_) ) { return 0; }
        if ( checkFile(tempName) ) { return 0; }

        if ( File::Exists(fileName_.c_str()) ) {
            File::Unlink(fileName_.c_str());
        }
        if ( File::Exists(tempName.c_str()) ) {
            File::Unlink(tempName.c_str());
        }

        std::auto_ptr< File > file( new File );
        file->RWCreate( tempName.c_str() );
        file->SetUnbuffered();

        // getting preferred size
        struct stat st;
        ::stat( tempName.c_str(), &st );
        if ( st.st_blksize < 4096 ) { st.st_blksize = 4096; }
        size_t chunkSize = ( packer_.blockSize() < unsigned(st.st_blksize) ?
                             (st.st_blksize - 1)/packer_.blockSize() : 0) + 1;

        offset_type writtenSize = 0;
        buffer_type buffer;
        buffer.resize( chunkSize * packer_.blockSize() );

        util::msectime_type startTime, currentTime;
        startTime = currentTime = util::currentTimeMillis();
        if (log_) {
            smsc_log_info(log_,"preallocation of %s has started, speed=%u kB/s",
                          fileName_.c_str(), unsigned(speed_));
        }
        MutexGuard mg(sleepMonitor_);
        offset_type nextBlock = offset_type(fileCount_) * fileSize_ * packer_.blockSize();
        size_t freeCount = fileSize_;
        buffer_type::iterator optr = buffer.begin();
        do {
            nextBlock += packer_.blockSize();
            BlockNavigation bn;
            bn.setFreeCells(--freeCount);
            if ( freeCount == 0 ) {
                bn.setNextBlock(packer_.notUsed());
            } else {
                bn.setNextBlock(nextBlock);
            }
            bn.savePtr(&buffer[optr-buffer.begin()]);
            optr += packer_.blockSize();
            if ( optr == buffer.end() || freeCount == 0 ) {
                // we have to write buffer
                size_t bufsize = optr - buffer.begin();
                file->Write(&buffer[0],bufsize);
                writtenSize += bufsize;
                if ( speed_ != 0 ) {
                    currentTime = util::currentTimeMillis();
                    util::msectime_type expectedTime = writtenSize / speed_;
                    util::msectime_type elapsedTime = currentTime - startTime;
                    if ( elapsedTime < expectedTime ) {
                        sleepMonitor_.wait(int(expectedTime-elapsedTime));
                        if ( stopping() ) {
                            if ( log_ ) {
                                smsc_log_warn(log_,"preallocation of %s has been externally stopped, freecount=%u",
                                              fileName_.c_str(), unsigned(freeCount));
                            }
                            file->Close();
                            return -1;
                        }
                    }
                }
                optr = buffer.begin();
            } // if write is needed
        } while ( freeCount != 0 );
        makeFreeChain();
        file->Rename(fileName_.c_str());
        currentTime = util::currentTimeMillis();
        if (log_) {
            smsc_log_info(log_,"preallocation of %s has finished in %u sec",
                          fileName_.c_str(), (currentTime - startTime)/1000 );
        }
        file_ = file;
        return 0;
    }


    bool checkFile( const std::string& fname )
    {
        if ( ! File::Exists(fname.c_str()) ) { return false; }
        std::auto_ptr< File > file(new File);
        try {
            file->RWOpen(fname.c_str());
            file->SetUnbuffered();
            BlockNavigation bn;
            bn.load(*file.get());
            if ( ! (bn.isFree() &&
                    bn.freeCells() == fileSize_-1 &&
                    bn.nextBlock() == (offset_type(fileCount_)*fileSize_+1)*packer_.blockSize()) ) {
                if (log_) {
                    smsc_log_warn(log_,"first block in preallocated file %s is wrong", fname.c_str());
                }
                return false;
            }
        } catch ( FileException& e ) {
            if (log_) {
                smsc_log_warn(log_,"cannto read first block in file %s", fname.c_str());
            }
        }
        try {
            file->Seek( offset_type(fileSize_-1)*packer_.blockSize() );
            BlockNavigation bn;
            bn.load(*file.get());
            if ( ! (bn.isFree() &&
                    bn.freeCells() == 0 &&
                    bn.nextBlock() == packer_.notUsed() ) ) {
                if (log_) {
                    smsc_log_warn(log_,"last block in preallocated file %s is wrong", fname.c_str());
                }
                return false;
            }
        } catch ( FileException& e ) {
            if (log_) {
                smsc_log_warn(log_,"cannot read last block in file %s",fname.c_str()); 
            }
            return false;
        }
        if (log_) {
            smsc_log_info(log_,"preallocated file %s seems to be ok", fname.c_str());
        }
        makeFreeChain();
        if ( fname != fileName_ ) {
            file->Rename(fileName_.c_str());
        }
        file_ = file;
        return true;
    }


    void makeFreeChain()
    {
        unsigned freeChainSize = std::min(fileSize_,size_t(1000U));
        freeChain_.reserve(freeChainSize);
        unsigned iend = unsigned(fileCount_*fileSize_+freeChainSize);
        for ( unsigned i = fileCount_ * fileSize_; i < iend; ++i ) {
            freeChain_.push_back(i);
        }
    }

private:
    smsc::core::synchronization::EventMonitor sleepMonitor_;
    std::string fileName_;
    HSPacker    packer_;
    size_t      fileSize_;
    size_t      fileCount_;
    size_t      speed_;     // kb/sec
    smsc::logger::Logger* log_;

    std::auto_ptr< smsc::core::buffers::File > file_;
    FreeChainType                              freeChain_;
};

/// ======================================================================


struct BlocksHSStorage2::StorageState
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

    std::string toString() const {
        char buf[100];
        snprintf(buf,sizeof(buf), "files=%u frees=%u ffb=%x", fileCount, freeCount, ffb);
        return buf;
    }

public:
    uint32_t fileCount;
    uint32_t freeCount;
    uint32_t ffb;
};


/// ======================================================================
struct BlocksHSStorage2::Transaction
{
public:
    bool operator < ( const Transaction& t ) const {
        return serial < t.serial;
    }

public:
    StorageState  oldState;
    StorageState  newState;
    buffer_type   oldBuf;
    buffer_type   newBuf;
    uint32_t      origSerial;
    uint32_t      serial;
    size_t        endsAt;     // in buffer
};


/// ======================================================================

class BlocksHSStorage2::TransApplier
{
public:
    TransApplier( BlocksHSStorage2& s, bool isNew ) :
    s_(s), isNew_(isNew) {}

    void operator () ( const Transaction* t ) {
        if (!t) return;
        try {
            s_.writeBlocks( isNew_ ? t->newBuf : t->oldBuf );
        } catch ( std::exception& e ) {
            throw smsc::util::Exception( "failed to apply trans #%u/%s: %s",
                                         t->origSerial, isNew_ ? "new" : "old",
                                         e.what() );
        }
    }

    void setState( const Transaction* t ) {
        const StorageState& state = isNew_ ? t->newState : t->oldState;
        if ( state.fileCount < s_.files_.size() ) {
            std::for_each( s_.files_.begin() + state.fileCount, s_.files_.end(), PtrDestroy() );
            s_.files_.erase( s_.files_.begin()+state.fileCount, s_.files_.end() );
        }
        s_.freeChain_.clear();
        s_.freeCount_ = state.freeCount;
        s_.freeChain_.push_back( state.ffb );
        if ( !s_.readFreeBlocks(1) ) {
            throw smsc::util::Exception("readFreeBlocks(%s) ret false", isNew_ ? "new" : "old");
        }
        if ( isNew_ ) {
            s_.journalFile_.Seek(t->endsAt);
            s_.journalWrites_ = t->origSerial;
        } else {
            s_.journalWrites_ = 0;
        }
    }
private:
    BlocksHSStorage2&  s_;
    bool               isNew_;
};

/// ======================================================================

class BlocksHSStorage2::FreeChainRescuer
{
public:
    FreeChainRescuer( BlocksHSStorage2& s ) : s_(s) {}
    inline void freeBlock( index_type idx ) {
        freeChain_.push_back( idx );
    }
    void freeBlocks( std::vector< offset_type >& offsets ) {
        if ( offsets.size() == 0 ) return;
        const size_t freeChainSize = freeChain_.size();
        freeChain_.resize( freeChain_.size() + offsets.size() );
        std::transform( offsets.begin(), offsets.end(),
                        freeChain_.begin() + freeChainSize,
                        std::bind2nd( std::divides<offset_type>(), s_.blockSize()) );
    }

    inline size_t freeCount() const { return freeChain_.size(); }

    inline index_type ffb() {
        assert( freeChain_.size() > 0 );
        std::sort( freeChain_.begin(), freeChain_.end() );
        freeChain_.erase( std::unique( freeChain_.begin(), freeChain_.end() ),
                          freeChain_.end() );
        return freeChain_.front(); 
    }

    ~FreeChainRescuer() 
    {
        try {
            size_t freeChainSize = freeChain_.size();
            if ( !freeChainSize ) { return; }
            // std::sort( freeChain_.begin(), freeChain_.end() );
            if ( s_.log_ ) {
                smsc_log_info(s_.log_,"writing a chain of size %u", unsigned(freeChainSize));
            }
            for ( std::vector< index_type >::const_iterator i = freeChain_.begin();
                  i != freeChain_.end();
                  ) {
                const offset_type offset = s_.idx2pos(*i);
                ++i;
                File* f = s_.getFile(offset);
                f->Seek(s_.getOffset(offset));
                BlockNavigation bn;
                bn.setFreeCells( --freeChainSize );
                bn.setNextBlock( freeChainSize == 0 ? s_.packer_.notUsed() : s_.idx2pos(*i) );
                bn.save(*f);
            }
        } catch ( std::exception& e ) {
            smsc_log_error(s_.log_,"exc in freeChainRescue: %s", e.what() );
        }
    }
private:
    BlocksHSStorage2&         s_;
    std::vector< index_type > freeChain_;
};

/// ======================================================================
bool BlocksHSStorage2::Scanner::next()
{
    if (!s_) return false;
    const index_type maxIdx = s_->fileSize_ * s_->files_.size();
    if (idx_==index_type(-1)) {
        idx_ = 0;
    } else if ( idx_ < maxIdx ){
        ++idx_;
    }
    while ( idx_ < maxIdx ) {
        offset_type offset = s_->idx2pos(idx_);
        File* f = s_->getFile(offset);
        if ( !f ) { break; }
        const off_t localOffset = s_->getOffset(offset);
        f->Seek(localOffset);
        BlockNavigation bn;
        bn.load(*f);
        if ( bn.isHead() ) return true;
        if ( bn.isFree() && f_ ) { f_->freeBlock(idx_); }
        ++idx_;
    }
    return false;
}



bool BlocksHSStorage2::Iterator::next()
{
    while ( scanner_.next() ) {
        if ( scanner_.s_->read(scanner_.idx_,buffer_) ) return true;
    }
    return false;
}



/// ======================================================================
BlocksHSStorage2::BlocksHSStorage2( DataFileManager& manager,
                                    smsc::logger::Logger* logger ) :
inited_(false),
packer_(1024,0/*,logger*/),
fileSize_(0),
fileSizeBytes_(0),
log_(logger),
oldBuf_(0),
newBuf_(0),
freeCount_(0),
journalWrites_(0),
maxJournalWrites_(100),
manager_(manager),
creationTask_(0)
{
    makeStaticData();
    rnd_.setSeed(time(0));
}

BlocksHSStorage2::~BlocksHSStorage2()
{
    // it is guaranteed that manager_ is already stopped and all tasks are also stopped.
    {
        MutexGuard mg(creationMutex_);
        if ( creationTask_.get() ) {
            creationTask_->waitUntilReleased();
        }
    }
    delete oldBuf_;
    delete newBuf_;
}



int BlocksHSStorage2::open( const std::string& dbname, const std::string& dbpath )
{
    if ( inited_ ) return ALREADY_INITED;
    if ( dbname.empty() ) return DBNAME_INVALID;
    dbname_ = dbname;
    dbpath_ = dbpath;
    int ret = doOpen();
    if (!ret) { inited_ = true; }
    return ret;
}


int BlocksHSStorage2::create( const std::string& dbname,
                              const std::string& dbpath,
                              size_t fileSize,
                              size_t blockSize,
                              uint32_t version )
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


int BlocksHSStorage2::recover( const std::string& dbname,
                               const std::string& dbpath,
                               IndexRescuer* indexRescuer,
                               uint32_t version )
{
    if (inited_) return ALREADY_INITED;
    if (dbname.empty()) return DBNAME_INVALID;
    version_ = version;
    dbname_ = dbname;
    dbpath_ = dbpath;
    int ret = doRecover( indexRescuer );
    if (!ret) { inited_ = true; }
    return ret;
}


BlocksHSStorage2::index_type BlocksHSStorage2::change( index_type   oldIndex,
                                                       buffer_type* oldBuf,
                                                       buffer_type* newBuf,
                                                       const KeyLogger* kl )
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
                    // FIXME: the method above should restore the previous state of the free chain and header?
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
                // FIXME: the method above should restore the previous state of storage?
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


/// reading used blocks at offset.
bool BlocksHSStorage2::read( offset_type  offset,
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


bool BlocksHSStorage2::popFreeBlocks( size_t       needBlocks,
                                      std::vector< offset_type >& affectedBlocks,
                                      buffer_type* buffer )
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
    // check if we have enough free blocks and start a creation task if not.
    checkFreeCount(freeCount_);
    return true;
}


bool BlocksHSStorage2::pushFreeBlocks( size_t freeStart,
                                       const std::vector< offset_type >& affectedBlocks,
                                       buffer_type* buffer )
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


int BlocksHSStorage2::doOpen()
{
    {
        // opening journal file
        const std::string fn = dbpath_ + "/" + dbname_ + ".jnl";
        try {
            journalFile_.RWOpen( fn.c_str() );
            journalFile_.SetUnbuffered();
            // reading the head
            journal_.resize(journalHeaderSize());
            journalFile_.Read(&journal_[0],journalHeaderSize());
            // deserialization
            Deserializer dsr(journal_);
            uint32_t val;
            dsr >> val;
            version_ = val;
            dsr >> val;
            packer_ = HSPacker(val,0/*,log_*/);
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
            smsc_log_info(log_,"journal header has been read: version=%x blockSize=%u fileSize=%u",
                          unsigned(version_), unsigned(packer_.blockSize()), unsigned(fileSize_));
        }

        // reading the whole journal file
        if ( journalFile_.Size() > 1000000 ) {
            smsc_log_error(log_,"journal file %s is too big", fn.c_str() );
            return JOURNAL_FILE_READ_FAILED;
        }
    }

    journal_.resize(journalFile_.Size());
    journalFile_.Read(&journal_[journalHeaderSize()],journalFile_.Size()-journalHeaderSize());
    journalWrites_ = 0;

    // reading transactions
    typedef std::vector< Transaction* > TransList;
    TransList transactions;
    transactions.reserve(maxJournalWrites_);
    for ( buffer_type::iterator iptr = journal_.begin() + journalHeaderSize();
          iptr < journal_.end(); ) {
          
        // search for starting position
        iptr = std::find( iptr, journal_.end(), transactionHeader[0]);
        if ( iptr+transactionHeader.size() >= journal_.end() ) {
            // eof reached
            break; 
        }
        if ( ! std::equal(transactionHeader.begin(), transactionHeader.end(), iptr) ) {
            // not equal
            ++iptr;
            continue;
        }

        buffer_type::iterator optr = iptr + transactionHeader.size();
        Transaction* t = 0;
        try {
            t = readJournal( journal_, optr );
        } catch ( std::exception& e ) {
            if (log_) {
                smsc_log_warn(log_,"exception reading jnl: %s", e.what());
            }
        }
        if ( t ) {
            iptr = optr;
            transactions.push_back( t );
            // checking if the eof-mark is here
            if ( iptr + transactionEnd.size() >= journal_.end() ) {
                // file has ended
                break;
            }
            if ( std::equal(transactionEnd.begin(), transactionEnd.end(), iptr) ) {
                // eof-mark found
                break;
            }
        } else {
            ++iptr;
        }
    }

    // journal data is not needed anymore
    buffer_type().swap(journal_);

    if ( transactions.size() == 0 ) {
        if (log_) {
            smsc_log_error(log_,"no transactions have be read");
        }
        return JOURNAL_FILE_READ_FAILED;
    }

    {
        // fixing serial numbers
        // shifting all transaction so that they will have sequential serial numbers
        uint32_t serialShift = ( transactions.front()->serial > 0x40000000U &&
                                 transactions.front()->serial < 0xc0000000U ) ?
            0U : 0x80000000U;
        for ( TransList::const_iterator i = transactions.begin();
              i != transactions.end();
              ++i ) {
            (*i)->serial += serialShift;
        }
    }

    int ret = 0;
    const char* transType = "none";
    do { // fake loop

        if ( transactions.size() > 1 ) {
            std::sort( transactions.begin(), transactions.end(), PtrLess() );
            // check that all transactions are sequential
            TransList::const_iterator i = transactions.begin();
            for ( TransList::const_iterator j = i+1;
                  j != transactions.end();
                  ++j ) {
                if ( (*i)->serial + 1 != (*j)->serial ) {
                    if (log_) {
                        smsc_log_error(log_,"we have non-sequential transactions: %u %u",
                                       (*i)->origSerial, (*j)->origSerial );
                    }
                    ret = JOURNAL_FILE_READ_FAILED;
                    break;
                }
                ++i;
            }
        }
        if ( ret ) break;

        // all transaction are sequential

        // opening files
        std::for_each( files_.begin(), files_.end(), PtrDestroy() );
        files_.clear();
        try {
            const size_t fileCount = transactions.back()->newState.fileCount;
            for ( size_t i = 0; i < fileCount; ++i ) {
                const std::string fn = makeFileName(i);
                if ( ! File::Exists(fn.c_str()) ) {
                    throw smsc::util::Exception("file %s does not exist", fn.c_str());
                }
                std::auto_ptr< File > file(new File);
                file->RWOpen(fn.c_str());
                file->SetUnbuffered();
                files_.push_back(file.release());
            }
        } catch ( std::exception& e ) {
            if (log_) {smsc_log_error(log_,"cannot open data files: %s", e.what());}
            ret = DATA_FILES_OPEN_FAILED;
            break;
        }
        
        if (log_) {
            smsc_log_info(log_,"all %u files have been opened",unsigned(files_.size()));
        }

        try {
            TransApplier applier(*this,true);
            std::for_each( transactions.begin(), transactions.end(), applier );
            applier.setState( transactions.back() );
            transType = "new";
            ret = 0;
            break;
        } catch ( std::exception& e ) {
            if (log_) {
                smsc_log_warn(log_,"new transactions: %s", e.what());
            }
        }

        // new trans failed
        
        // new state is broken, trying to rollback all transactions
        if (log_) {
            smsc_log_warn(log_,"could not apply new transactions, trying with old");
        }
        try {
            TransApplier applier(*this,false);
            std::for_each( transactions.rbegin(), transactions.rend(), applier );
            applier.setState( transactions.front() );
            transType = "old";
            ret = 0;
        } catch ( std::exception& e ) {
            if (log_) { smsc_log_warn(log_,"old transactions: %s", e.what()); }
            ret = TRANSACTION_WRITE_FAILED;
        }

    } while ( false );

    // destroying all transactions
    std::for_each( transactions.begin(), transactions.end(), PtrDestroy() );

    if ( ret == 0 ) {
        if (log_) {
            smsc_log_info(log_,"storage has been opened via %s trans: version=%x blockSize=%u/%x fileSize=%u/%x fszBytes=%llx fileCount=%u freeCount=%u ffb=%llx",
                          transType,
                          unsigned(version_), unsigned(packer_.blockSize()), unsigned(packer_.blockSize()),
                          unsigned(fileSize_), unsigned(fileSize_), fileSizeBytes_,
                          unsigned(files_.size()), unsigned(freeCount_), idx2pos(freeChain_.front()));
        }
        checkFreeCount(freeCount_);
    }
    return ret;
}


int BlocksHSStorage2::doCreate()
{
    const std::string fn = dbpath_ + "/" + dbname_ + ".jnl";
    if ( File::Exists(fn.c_str()) ) {
        return JOURNAL_FILE_ALREADY_EXISTS;
    }
    try {
        std::string dfn = makeFileName(0);
        if ( File::Exists(dfn.c_str())) {
            throw smsc::util::Exception("there is file %s already", dfn.c_str());
        }
        journalFile_.RWCreate(fn.c_str());
        journalFile_.SetUnbuffered();
    } catch ( std::exception& e ) {
        if (log_) {
            smsc_log_error(log_,"exc in create: %s", e.what());
        }
        return JOURNAL_FILE_CREATION_FAILED;
    }

    try {
        // create a data file
        if ( ! attachNewFile() ) {
            throw smsc::util::Exception("return false");
        }
    } catch ( std::exception& e ) {
        if (log_) {
            smsc_log_error(log_,"cannot create: %s", e.what());
        }
        return DATA_FILES_OPEN_FAILED;
    }
    try {
        // attach the data file
        checkFreeCount(freeCount_);
    } catch ( std::exception& e ) {
        if (log_) {
            smsc_log_error(log_,"cannot load initial free chain");
        }
        return FREE_CHAIN_BROKEN;
    }
    if (log_) {
        smsc_log_info(log_,"storage has been created: version=%x blockSize=%u/%x fileSize=%u/%x fszBytes=%llx fileCount=%u freeCount=%u ffb=%llx",
                      unsigned(version_), unsigned(packer_.blockSize()), unsigned(packer_.blockSize()),
                      unsigned(fileSize_), unsigned(fileSize_), fileSizeBytes_,
                      unsigned(files_.size()), unsigned(freeCount_), idx2pos(freeChain_.front()));
    }
    return 0;
}


int BlocksHSStorage2::doRecover( IndexRescuer* indexRescuer )
{
    std::for_each( files_.begin(), files_.end(), PtrDestroy() );
    files_.clear();

    // first of all open all existing data files
    fileSizeBytes_ = 0;
    for ( size_t i = 0; ; ++i ) {
        const std::string fn = makeFileName(i);
        if ( ! File::Exists(fn.c_str()) ) break;
        try {
            if ( i == 0 ) {
                fileSizeBytes_ = File::Size(fn.c_str());
                if ( fileSizeBytes_ == 0 ) {
                    throw smsc::util::Exception("first file has size=0",fn.c_str());
                }
            } else {
                offset_type fsz = offset_type(File::Size(fn.c_str()));
                if ( fileSizeBytes_ != fsz ) {
                    throw smsc::util::Exception("filesize mismatch %llx != %llx",
                                                fn.c_str(), fsz, fileSizeBytes_ );
                }
            }
            std::auto_ptr< File > file(new File);
            file->RWOpen(fn.c_str());
            file->SetUnbuffered();
            files_.push_back(file.release());
        } catch ( std::exception& e ) {
            if (log_) {
                smsc_log_warn(log_,"exc open file %s: %s", fn.c_str(), e.what());
            }
            break;
        }
    }

    if ( files_.size() == 0 ) {
        if (log_) {
            smsc_log_error(log_,"no files found for dbpath=%s", dbpath_.c_str());
        }
        return DATA_FILES_OPEN_FAILED;
    }

    // then determine blockSize
    offset_type pos = 0;
    do {
        File* f = getFile(pos);
        f->Seek(getOffset(pos));
        BlockNavigation bn;
        bn.load( *f );
        pos = bn.nextBlock();
        if ( packer_.isNotUsed(pos) ) {
            // blockSize found
            size_t blockSize = (pos & ~BlockNavigation::badBit());
            packer_ = HSPacker(blockSize,0/*,log_*/);
            fileSize_ = fileSizeBytes_ / packer_.blockSize();
            fileSizeBytes_ = fileSize_ * packer_.blockSize();
            break;
        }
    } while (true);

    if (log_) {
        smsc_log_info(log_,"sizes recovered: blockSize=%u/%x fileSize=%u/%x fszBytes=%x files=%u",
                      unsigned(packer_.blockSize()),
                      unsigned(packer_.blockSize()),
                      unsigned(fileSize_),
                      unsigned(fileSize_),
                      unsigned(fileSizeBytes_),
                      unsigned(files_.size()));
    }
    
    const std::string fn = makeJnlFileName();
    try {
        if ( File::Exists(fn.c_str()) ) {
            File::Rename(fn.c_str(),(fn + ".bak").c_str());
        }
        journalFile_.RWCreate(fn.c_str());
        journalFile_.SetUnbuffered();
        journalWrites_ = 0; // recreate header at the next write
    } catch ( std::exception& e ) {
        if (log_) {
            smsc_log_error(log_,"cannot create journal file %s", fn.c_str());
        }
        return JOURNAL_FILE_CREATION_FAILED;
    }

    FreeChainRescuer freeChainer(*this);
    Scanner scanner(this,&freeChainer);
    buffer_type buffer;
    std::vector< offset_type > affected;
    while ( scanner.next() ) {
        buffer.clear();
        affected.clear();
        try {
            if ( ! read(idx2pos(scanner.idx_),buffer,&affected) ) {
                throw smsc::util::Exception("return false");
            }
            // recover index
            if ( indexRescuer ) {
                indexRescuer->recoverIndex(scanner.idx_,buffer);
            }
        } catch ( std::exception& e ) {
            // failed
            if (log_) {
                smsc_log_warn(log_,"failed to read used chain at %llx: %s",
                              idx2pos(scanner.idx_), e.what());
            }
            // free affected blocks
            freeChainer.freeBlocks( affected );
        }
    }
    
    // writing free chain information
    freeCount_ = freeChainer.freeCount();
    freeChain_.clear();
    if (freeCount_ > 0) freeChain_.push_back(freeChainer.ffb());
    buffer_type dummy;
    journalWrites_ = 0;
    if ( !writeJournal(dummy,dummy,StorageState(*this)) ) {
        return TRANSACTION_WRITE_FAILED;
    }
    return 0;
}


size_t BlocksHSStorage2::minTransactionSize() const
{
    static const size_t constantBufSize =
        4*2 +                             // length*2
        8*2 +                             // csum*2
        4 +                               // serial
        StorageState::dataSize()*2 +      // old+new
        4 +                               // oldbufsize
        4;                                // newbufsize
    return constantBufSize;
}


BlocksHSStorage2::Transaction* BlocksHSStorage2::readJournal( buffer_type& buffer,
                                                              buffer_type::iterator& iptr )
{
    const unsigned char* ptr = &buffer[iptr-buffer.begin()];
    // reading length
    uint32_t val;
    ptr = memscpy(&val,ptr,4);
    const size_t transactionSize = ntohl(val);
    if ( transactionSize < minTransactionSize() ) {
        throw smsc::util::Exception("weird transLen=%u, must be >= minLen=%u",
                                    unsigned(transactionSize),
                                    unsigned(minTransactionSize()) );
    }
    if ( iptr + transactionSize > buffer.end() ) {
        throw smsc::util::Exception("weird transLen=%u, must be <= %u",
                                    unsigned(transactionSize),
                                    unsigned(buffer.end()-iptr));
    }

    // checking csums
    uint64_t csum1, csum2;
    uint32_t ctrlval;
    ptr = memscpy( &csum1, ptr, 8);
    if ( csum1 == 0 ) {
        throw smsc::util::Exception("control sum cannot be 0");
    }
    {
        const unsigned char* endptr = &buffer[(iptr - buffer.begin()) + transactionSize - 12];
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
    Transaction* t = new Transaction;
    ptr = memscpy(&val,ptr,4);
    t->serial = t->origSerial = ntohl(val);
    ptr = t->oldState.loadData(ptr);
    ptr = t->newState.loadData(ptr);
    uint32_t szval;
    ptr = memscpy(&szval,ptr,4);
    szval = ntohl(szval);
    t->oldBuf.resize(szval);
    if (szval > 0 ) ptr = memscpy(&(t->oldBuf[0]),ptr,szval);
    ptr = memscpy(&szval,ptr,4);
    szval = ntohl(szval);
    t->newBuf.resize(szval);
    if (szval > 0 ) ptr = memscpy(&(t->newBuf[0]),ptr,szval);
    iptr += transactionSize;
    t->endsAt = (iptr - buffer.begin());
    if (log_) {
        smsc_log_debug(log_,"transaction #%u has been read: old=(%s,sz=%u) new=(%s,sz=%u) ends=%u",
                       t->serial,
                       t->oldState.toString().c_str(),
                       unsigned(t->oldBuf.size()),
                       t->newState.toString().c_str(),
                       unsigned(t->newBuf.size()),
                       unsigned(t->endsAt) );
    }
    return t;
}


bool BlocksHSStorage2::writeJournal( const buffer_type& oldbuf,
                                     const buffer_type& newbuf,
                                     const StorageState& oldhead )
{
    bool needHeader = ( journalWrites_ == 0 );
    ++journalWrites_;
    if ( journalWrites_ == 0 ) { ++journalWrites_; }
    const bool lastTrans = (journalWrites_ % maxJournalWrites_) == 0;

    const uint32_t jnlSize = minTransactionSize() + oldbuf.size() + newbuf.size();
    journal_.resize( ( needHeader ? journalHeaderSize() : 0 ) +
                     transactionHeader.size() + jnlSize +
                     ( lastTrans ? transactionEnd.size() : 0 ) );
    unsigned char* ptr = &journal_[0];
    const uint32_t jnlSizeNet = htonl(jnlSize);

    StorageState newhead(*this);

    // composing buffer
    if ( needHeader ) {
        // make the header
        uint32_t val = htonl(version_);
        ptr = mempcpy(ptr,&val,4);
        val = htonl(packer_.blockSize());
        ptr = mempcpy(ptr,&val,4);
        val = htonl(fileSize_);
        ptr = mempcpy(ptr,&val,4);
    }

    // control sum
    uint64_t csum;
    do {
        csum = rnd_.getNextNumber();
    } while ( csum == 0 );
    // timestamp -- not written
    // cvt.set( int64_t(util::currentTimeMillis()) );
    // const uint64_t tstamp = cvt.cvt.quads[0];
    ptr = mempcpy(ptr,&transactionHeader[0],transactionHeader.size());
    ptr = mempcpy(ptr,&jnlSizeNet,4);
    ptr = mempcpy(ptr,&csum,8);
    const uint32_t serial = htonl(journalWrites_);
    // ptr = mempcpy(ptr,&tstamp,8);
    ptr = mempcpy(ptr,&serial,4);
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

    if ( lastTrans ) {
        ptr = mempcpy(ptr,&transactionEnd[0],transactionEnd.size());
    }

    assert( ptr == (&journal_[0] + journal_.size()) );

    if ( log_ ) {
        smsc_log_debug(log_,"writing journal #%u of sz=%u csum=%llx",
                       unsigned(journalWrites_),
                       unsigned(jnlSize),
                       csum );
    }
    if ( needHeader ) { journalFile_.Seek(0); }
    journalFile_.Write( &journal_[0], journal_.size() );
    if ( lastTrans ) {
        journalFile_.Truncate(journalFile_.Pos());
        journalFile_.Seek(journalHeaderSize());
    }
    return true;
}


bool BlocksHSStorage2::writeBlocks( const buffer_type& buffer,
                                    const std::vector< offset_type >* posAndSize,
                                    size_t initialPos )
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


bool BlocksHSStorage2::readFreeBlocks( size_t needBlocks, bool allowNewFile )
{
    const size_t freeChainSize = freeChain_.size();
    assert( ! freeChain_.empty() );
    offset_type lastBlock = idx2pos(freeChain_.back());
    bool rv = false;
    // hardcoded value
    size_t counter = needBlocks + 100;
    if ( needBlocks >= freeCount_ ) {
        checkFreeCount(0);
    } else {
        checkFreeCount(freeCount_-needBlocks);
    }
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

        // FIXME: add a check for freeCells()
        
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


void BlocksHSStorage2::checkFreeCount( size_t freeCount )
{
    if ( freeCount > manager_.creationThreshold() ) { return; }
    if ( creationTask_.get() ) { return; }

    // we have to create a new task
    size_t speed = 0;
    {
        MutexGuard mg(creationMutex_);
        if ( creationTask_.get() ) { return; }

        // calculate speed (kb/sec)
        if ( manager_.getExpectedSpeed() > 0 && freeCount > 0 ) {
            speed = (fileSizeBytes_/1024) * manager_.getExpectedSpeed() / freeCount; 
        }
        creationTask_.reset( new CreationTask( makeFileName(files_.size()),
                                               packer_.blockSize(),
                                               fileSize_,
                                               files_.size(),
                                               speed,
                                               log_ ));
    }

    if ( inited_ ) {
        manager_.startTask( creationTask_.get(), false );
    } else {
        // invoked on the same thread
        try {
            creationTask_->Execute();
            creationTask_->onRelease();
        } catch ( std::exception& e ) {
            if (log_) {
                smsc_log_warn(log_,"exc in creationTask: %s", e.what());
            }
        }
    }
}


bool BlocksHSStorage2::attachNewFile()
{
    if ( ! creationTask_.get() ) {
        // there is no creation task ?
        checkFreeCount(0);
    }
    MutexGuard mg(creationMutex_);
    if ( ! creationTask_.get() ) {
        return false;
    }
    // creationTask_->waitUntilStarted();
    creationTask_->waitUntilReleased();
    // creation task is finished
    std::auto_ptr<CreationTask> ct(creationTask_.release());
    File* f = ct->getFile();
    if ( !f ) { return false; }
    // write a new transaction
    StorageState oldState(*this);
    files_.push_back(f);
    const size_t freeChainSize = freeChain_.size();
    freeChain_.insert(freeChain_.end(),
                      ct->freeChain().begin(),
                      ct->freeChain().end());
    freeCount_ += ct->fileSize();
    bool rv = false;
    try {
        buffer_type dummy;
        if ( writeJournal(dummy,dummy,oldState) ) {
            rv = true;
        } else {
            throw smsc::util::Exception("writeJournal returns false");
        }
    } catch ( std::exception& e ) {
        if (log_) {
            smsc_log_warn(log_,"attachNewFile(#%u): %s", unsigned(files_.size())-1, e.what());
        }
    }
    if ( ! rv ) {
        // recovery
        freeChain_.erase(freeChain_.begin()+freeChainSize,freeChain_.end());
        freeCount_ -= ct->fileSize();
        files_.pop_back();
        delete f;
    }
    return rv;
}


/*
int BlocksHSStorage2::writeJournalHeader()
{
    const std::string fn = dbpath_ + "/" + dbname_ + ".jnl";
    if ( File::Exists(fn.c_str()) ) {
        return JOURNAL_FILE_ALREADY_EXISTS;
    }
    try {
        journalFile_.RWCreate( fn.c_str() );
        journalFile_.SetUnbuffered();
        journal_.clear();
        journal_.reserve(journalHeaderSize());
        Serializer ser(journal_);
        ser << uint32_t(version_);
        ser << uint32_t(packer_.blockSize());
        ser << uint32_t(fileSize_);
        assert( journal_.size() == journalHeaderSize());
        journalFile_.Write(&journal_[0],journalHeaderSize());

    } catch ( std::exception& e ) {
        if (log_) {
            smsc_log_error(log_,"cannot create journal file %s: %s", fn.c_str(), e.what());
        }
        return JOURNAL_FILE_CREATION_FAILED;
    }
    return 0;
}
 */

} // namespace storage
} // namespace util
} // namespace scag2
