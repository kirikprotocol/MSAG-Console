#include <algorithm>
#include <cmath>
#include "BlocksHSStorage2.h"
#include "scag/util/WatchedThreadedTask.h"
#include "DataFileManager.h"
#include "scag/util/Time.h"
#include "util/PtrDestroy.h"

using namespace smsc::core::buffers;

namespace {

bool staticDone = false;
std::string journalRecordMark;
std::string journalRecordTrailer;
smsc::core::synchronization::Mutex staticMutex;

void makeStaticData()
{
    if ( staticDone ) return;
    MutexGuard mg(staticMutex);
    if ( staticDone ) return;
    journalRecordMark = "TrAnSaCt";
    journalRecordTrailer = "EnDoFtRaNsAcTiOnS";
    staticDone = true;
}

scag2::util::storage::DummyKeyLogger dummyKeyLogger;

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
        unsigned freeChainSize = unsigned(std::min(fileSize_,size_t(20000U)));
        freeChain_.reserve(freeChainSize);
        unsigned iend = unsigned(fileCount_*fileSize_+freeChainSize);
        for ( unsigned i = unsigned(fileCount_ * fileSize_); i < iend; ++i ) {
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
    fileCount(unsigned(bhs.files_.size())),
    freeCount(unsigned(bhs.freeCount_)),
    ffb(unsigned(bhs.freeChain_.empty() ? bhs.packer_.invalidIndex() : bhs.freeChain_.front())) {}

    static size_t dataSize() { return 12; }

    char* saveData( char* ptr ) const
    {
        io::EndianConverter::set32(ptr,fileCount);
        io::EndianConverter::set32(ptr+4,freeCount);
        io::EndianConverter::set32(ptr+8,ffb);
        return ptr+12;
    }

    const char* loadData( const char* ptr )
    {
        fileCount = io::EndianConverter::get32(ptr);
        freeCount = io::EndianConverter::get32(ptr+4);
        ffb = io::EndianConverter::get32(ptr+8);
        return ptr+12;
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
struct BlocksHSStorage2::Transaction : public JournalRecord
{
public:
    Transaction() : oldBufSize(0), newBufSize(0) {}

    Transaction( const BlocksHSStorage2& bh ) :
    oldState(bh), oldBufSize(0), newBufSize(0) {}

    virtual void load( const void* p, size_t bufsize ) {
        const char* ptr = reinterpret_cast<const char*>(p);
        const char* endptr = ptr + bufsize;
        ptr = oldState.loadData(ptr);
        ptr = newState.loadData(ptr);
        oldBufSize = io::EndianConverter::get32(ptr);
        ptr += 4;
        if ( ptr + oldBufSize > endptr ) {
            throw smsc::util::Exception("buffer underrun");
        }
        oldBuf = ptr;
        ptr += oldBufSize;
        newBufSize = io::EndianConverter::get32(ptr);
        ptr += 4;
        if ( ptr + newBufSize != endptr ) {
            throw smsc::util::Exception("buffer size mismatch");
        }
        newBuf = ptr;
    }
    virtual void save( void* p ) const {
        char* ptr = reinterpret_cast<char*>(p);
        ptr = oldState.saveData(ptr);
        ptr = newState.saveData(ptr);
        io::EndianConverter::set32(ptr,uint32_t(oldBufSize));
        ptr += 4;
        if ( oldBufSize > 0 ) {
            memcpy(ptr,oldBuf,oldBufSize);
            ptr += oldBufSize;
        }
        io::EndianConverter::set32(ptr,uint32_t(newBufSize));
        ptr += 4;
        if ( newBufSize > 0 ) {
            memcpy(ptr,newBuf,newBufSize);
            ptr += newBufSize;
        }
    }
    virtual size_t savedDataSize() const {
        return 2*(4+StorageState::dataSize()) + oldBufSize + newBufSize;
    }

    virtual std::string toString() const
    {
        char buf[200];
        snprintf(buf,sizeof(buf)," oldSt=%s newSt=%s oldSz=%u newSz=%u",
                 oldState.toString().c_str(),
                 newState.toString().c_str(),
                 unsigned(oldBufSize),
                 unsigned(newBufSize) );
        return JournalRecord::toString() + buf;
    }

    StorageState         oldState;
    const void*          oldBuf;    // not owned
    size_t               oldBufSize;

    StorageState         newState;
    const void*          newBuf;    // not owned
    size_t               newBufSize;
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
                smsc_log_info(s_.log_,"writing a chain of size %u, ffb=%llx",
                              unsigned(freeChainSize),
                              s_.packer_.idx2pos(freeChain_.front()));
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
journalFile_(*this,100,logger),
manager_(manager),
creationTask_(0),
keylogger_(&::dummyKeyLogger),
readonly_(false)
{
    makeStaticData();
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
    std::for_each( files_.rbegin(), files_.rend(), smsc::util::PtrDestroy() );
}



int BlocksHSStorage2::open( const std::string& dbname,
                            const std::string& dbpath,
                            bool readonly )
{
    if ( inited_ ) return ALREADY_INITED;
    if ( dbname.empty() ) return DBNAME_INVALID;
    dbname_ = dbname;
    dbpath_ = dbpath;
    readonly_ = readonly;
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
    readonly_ = false;
    packer_ = HSPacker(blockSize,0/*,log_*/);
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
    readonly_ = false;
    int ret = doRecover( indexRescuer );
    if (!ret) { inited_ = true; }
    return ret;
}


BlocksHSStorage2::index_type BlocksHSStorage2::change( index_type   oldIndex,
                                                       buffer_type* oldBuf,
                                                       buffer_type* newBuf )
{
    size_t needBlocks;
    size_t hasBlocks;
    size_t newDataSize = 0;
    size_t oldDataSize = 0;
    std::vector< offset_type > affectedBlocks;  // offsets
    const offset_type oldPos = idx2pos(oldIndex);

    Transaction transaction(*this);

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

                if ( !oldBuf ) {
                    if (!oldBuf_) oldBuf_ = new buffer_type;
                    oldBuf = oldBuf_;
                    oldBuf->clear();
                    // reread from disk
                    if (!read(oldPos,*oldBuf,&affectedBlocks)) break;
                } else if ( !getAffectedBlocks(*oldBuf,affectedBlocks) ) {
                    // buffer parsing failed
                    if (!oldBuf_) oldBuf_ = new buffer_type;
                    oldBuf = oldBuf_;
                    oldBuf->clear();
                    if (!read(oldPos,*oldBuf,&affectedBlocks)) break;
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
                break;
            }
        }

        // writing journal
        try {
            transaction.oldBuf = &(*oldBuf)[0];
            transaction.oldBufSize = oldBuf->size();
            transaction.newBuf = &(*newBuf)[0];
            transaction.newBufSize = newBuf->size();
            transaction.newState = StorageState(*this);
            journalFile_.writeRecord( transaction );
        } catch (...) {
            // cannot save transaction
            rollbackFreeChain(hasBlocks,needBlocks,affectedBlocks);
            break;
        }

        // calculating position-and-size of blocks to write
        std::vector< offset_type > posAndSize;
        packer_.offsetsToPosAndSize( affectedBlocks, newDataSize, posAndSize );

        // saving blocks
        if ( !writeBlocks(&(*newBuf)[0],newBuf->size(),&posAndSize) ) {
            // failure
            writeBlocks(&(*oldBuf)[0],oldBuf->size());
            rollbackFreeChain(hasBlocks,needBlocks,affectedBlocks);
            break;
        }

        ok = true;

    } while ( false );

    if ( ! ok ) {
        // rollback profile should be done externally
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
        BlockNavigation::saveIdx(ptr,*iter++);
        ptr += idxSize();
        BlockNavigation bn;
        size_t freeCount = freeCount_;
        while ( freeCount <= needBlocks ) { freeCount += fileSize_; }
        for ( size_t i = 0; i < needBlocks; ++i ) {
            bn.setFreeCells(--freeCount);
            bn.setNextBlock(*iter++);
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
            smsc_log_fatal(log_,"logic error: too few affected blocks in pushFree");
        }
        abort();
    }
    if ( freeChain_.empty() ) {
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
        BlockNavigation::saveIdx(ptr,*iter++);
        ptr += idxSize();
        BlockNavigation bn;
        size_t freeCount = freeCount_;
        while ( freeCount <= returnedBlocks ) { freeCount += fileSize_; }
        for ( size_t i = 0; i < returnedBlocks; ++i ) {
            bn.setFreeCells(--freeCount);
            if ( iter == affectedBlocks.end() ) {
                bn.setNextBlock(idx2pos(freeChain_.front()));
            } else {
                bn.setNextBlock(*iter++);
            }
            bn.savePtr(ptr);
            ptr += navSize();
        }
    }
    freeChain_.insert( freeChain_.begin(), temp.begin(), temp.end() );
    if ( freeChain_.size() > fileSize_ ) {
        freeChain_.erase(freeChain_.begin()+fileSize_,freeChain_.end());
    }
    freeCount_ += returnedBlocks;
    return true;
}


int BlocksHSStorage2::doOpen()
{
    try {
        std::for_each( files_.rbegin(), files_.rend(), smsc::util::PtrDestroy() );
        files_.clear();
        journalFile_.open( readonly_ );
        if ( files_.size() == 0 ) {
            throw smsc::util::Exception("no records found");
        }
    } catch ( std::exception& e ) {
        if (log_) {
            smsc_log_warn(log_,"cannot open journal file %s: %s",
                          journalFileName().c_str(), e.what() );
        }
        return JOURNAL_FILE_OPEN_FAILED;
    }

    if (log_) {
        smsc_log_info(log_,"storage has been opened: version=%x blockSize=%u/%x fileSize=%u/%x fszBytes=%llx fileCount=%u freeCount=%u ffb=%llx recSerial=%x",
                      unsigned(version_), unsigned(packer_.blockSize()), unsigned(packer_.blockSize()),
                      unsigned(fileSize_), unsigned(fileSize_), fileSizeBytes_,
                      unsigned(files_.size()), unsigned(freeCount_), idx2pos(freeChain_.front()),
                      unsigned(journalFile_.getRecordSerial()));
    }
    checkFreeCount(freeCount_);
    return 0;
}


int BlocksHSStorage2::doCreate()
{
    try {
        std::string dfn = makeFileName(0);
        if ( File::Exists(dfn.c_str())) {
            throw smsc::util::Exception("there is file %s already", dfn.c_str());
        }
        journalFile_.create();
    } catch ( std::exception& e ) {
        if (log_) {
            smsc_log_error(log_,"exc in create: %s", e.what());
        }
        return JOURNAL_FILE_CREATION_FAILED;
    }

    try {
        // create a data file
        if ( ! attachNewFile() ) {
            throw smsc::util::Exception("attach returns false");
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
        smsc_log_info(log_,"storage has been created: version=%x blockSize=%u/%x fileSize=%u/%x fszBytes=%llx fileCount=%u freeCount=%u ffb=%llx recSerial=%x",
                      unsigned(version_), unsigned(packer_.blockSize()), unsigned(packer_.blockSize()),
                      unsigned(fileSize_), unsigned(fileSize_), fileSizeBytes_,
                      unsigned(files_.size()), unsigned(freeCount_), idx2pos(freeChain_.front()),
                      unsigned(journalFile_.getRecordSerial()));
    }
    return 0;
}


int BlocksHSStorage2::doRecover( IndexRescuer* indexRescuer )
{
    std::for_each( files_.rbegin(), files_.rend(), smsc::util::PtrDestroy() );
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
    
    try {
        journalFile_.recreate();
    } catch ( std::exception& e ) {
        if (log_) {
            smsc_log_error(log_,"cannot recreate journal file %s", journalFileName().c_str());
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
                throw smsc::util::Exception("read() returns false");
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
    if (freeCount_ > 0) freeChain_.push_back(unsigned(freeChainer.ffb()));
    Transaction trans(*this);
    trans.newState = StorageState(*this);
    try {
        journalFile_.writeRecord(trans);
    } catch (...) {
        return TRANSACTION_WRITE_FAILED;
    }
    return 0;
}


bool BlocksHSStorage2::writeBlocks( const void* buffer,
                                    size_t bufferSize,
                                    const std::vector< offset_type >* posAndSize,
                                    size_t initialPos )
{
    if ( bufferSize < initialPos + idxSize() + navSize() ) { return true; }
    if ( ! posAndSize ) {
        posAndSize = &posAndSize_;
        packer_.extractBlocks( buffer, bufferSize, posAndSize_, packer_.notUsed(), initialPos );
    }
    // writing blocks
    assert( posAndSize->size() % 2 == 0 );
    const unsigned char* ptr = 
        reinterpret_cast<const unsigned char*>(buffer) + initialPos;
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
        freeChain_.push_back(unsigned(pos2idx(lastBlock)));
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
    if ( !inited_ ) {
        if ( freeCount > 100 ) { return; }
        freeCount = 0; // we want to create at maximum speed
    }

    // we have to create a new task
    size_t speed = 0;
    {
        MutexGuard mg(creationMutex_);
        if ( creationTask_.get() ) { return; }

        // calculate speed (kb/sec)
        if ( manager_.getExpectedSpeed() > 0 && freeCount > 0 ) {
            const size_t expectedSpeed = 
                (fileSizeBytes_/1024) * manager_.getExpectedSpeed() / manager_.creationThreshold();
            speed = expectedSpeed * 4 / 3;
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
    if ( ! creationTask_->released() ) {
        if (log_) {smsc_log_warn(log_,"waiting until preallocation task is finished");}
        creationTask_->waitUntilReleased();
    }
    // creation task is finished
    std::auto_ptr<CreationTask> ct(creationTask_.release());
    std::auto_ptr<File> f(ct->getFile());
    if ( !f.get() ) { return false; }
    // write a new transaction
    Transaction trans(*this);
    files_.push_back(f.release());
    const size_t oldFreeCount = freeCount_;
    const size_t freeChainSize = freeChain_.size();
    // overwrite the content of the last block in previous free chain
    bool rv = true;
    try {

        buffer_type oldBuf, newBuf;
        if ( ! freeChain_.empty() ) {
            // we have to overwrite the last free block pointer
            const offset_type lastBlock = idx2pos(freeChain_.back());
            oldBuf.resize(idxSize()+navSize());
            BlockNavigation::saveIdx(&oldBuf[0],lastBlock);
            BlockNavigation bn;
            bn.setFreeCells(0);
            bn.setNextBlock(packer_.notUsed());
            bn.savePtr(&oldBuf[idxSize()]);
            newBuf = oldBuf;
            bn.setFreeCells(ct->fileSize());
            bn.setNextBlock(idx2pos(ct->freeChain().front()));
            bn.savePtr(&newBuf[idxSize()]);
            trans.oldBuf = &oldBuf[0];
            trans.newBuf = &newBuf[0];
            trans.oldBufSize = trans.newBufSize = oldBuf.size();
            rv = writeBlocks(&newBuf[0], newBuf.size());
        }

        if ( rv ) {
            freeChain_.insert(freeChain_.end(),
                              ct->freeChain().begin(),
                              ct->freeChain().end());
            freeCount_ += ct->fileSize();
            trans.newState = StorageState(*this);
            journalFile_.writeRecord( trans );
        }

    } catch ( std::exception& e ) {
        if (log_) {
            smsc_log_warn(log_,"attachNewFile(#%u): %s", unsigned(files_.size())-1, e.what());
        }
        rv = false;
    }
    if ( ! rv ) {
        // recovery
        freeChain_.erase(freeChain_.begin()+freeChainSize,freeChain_.end());
        freeCount_ = oldFreeCount;
        f.reset(files_.back());
        files_.pop_back();
        // files_.pop_back();
        // delete f;
    }
    return rv;
}


// --- journal interface
const std::string& BlocksHSStorage2::journalRecordMark() const
{
    return ::journalRecordMark;
}
const std::string& BlocksHSStorage2::journalRecordTrailer() const
{
    return ::journalRecordTrailer;
}


void BlocksHSStorage2::saveJournalHeader( void* p ) const
{
    // make the header
    register char* ptr = reinterpret_cast<char*>(p);
    io::EndianConverter::set32(ptr,version_);
    io::EndianConverter::set32(ptr+4,uint32_t(packer_.blockSize()));
    io::EndianConverter::set32(ptr+8,uint32_t(fileSize_));
}


size_t BlocksHSStorage2::loadJournalHeader( const void* p )
{
    register const char* ptr = reinterpret_cast<const char*>(p);
    version_ = io::EndianConverter::get32(ptr);
    if ( version_ != 0x80000002 ) {
        throw smsc::util::Exception("wrong version %u", version_);
    }
    uint32_t bsz = io::EndianConverter::get32(ptr+4);
    packer_ = HSPacker(bsz,0/*,log_*/);
    fileSize_ = io::EndianConverter::get32(ptr+8);
    fileSizeBytes_ = offset_type(fileSize_) * packer_.blockSize();
    if (log_) {
        smsc_log_debug(log_,"journal header has been loaded: blockSize=%u/%x fileSize=%u/%x fszBytes=%llx",
                       unsigned(bsz), unsigned(bsz),
                       unsigned(fileSize_), unsigned(fileSize_),
                       uint64_t(fileSizeBytes_) );
    }
    return journalHeaderSize();
}


JournalRecord* BlocksHSStorage2::createJournalRecord()
{
    return new Transaction;
}


void BlocksHSStorage2::prepareForApplication( const std::vector< JournalRecord* >& records )
{
    // closing all files
    std::for_each( files_.rbegin(), files_.rend(), smsc::util::PtrDestroy() );
    files_.clear();

    // taking the newest state, opening all necessary files
    const Transaction* t = static_cast<const Transaction*>(records.back());
    const size_t fileCount = t->newState.fileCount;
    for ( size_t i = 0; i < fileCount; ++i ) {
        const std::string fn = makeFileName(i);
        if ( ! File::Exists(fn.c_str()) ) {
            throw smsc::util::Exception("file %s does not exist", fn.c_str());
        }
        std::auto_ptr< File > file(new File);
        if ( readonly_ ) {
            file->ROpen(fn.c_str());
        } else {
            file->RWOpen(fn.c_str());
        }
        file->SetUnbuffered();
        files_.push_back(file.release());
    }
    if (log_) {
        smsc_log_debug(log_,"all %u files opened", unsigned(files_.size()));
    }
}


void BlocksHSStorage2::applyJournalData( const JournalRecord& rec, bool takeNew )
{
    if ( readonly_ ) return;
    const Transaction& t = static_cast<const Transaction&>(rec);
    try {
        if ( takeNew ) {
            writeBlocks(t.newBuf,t.newBufSize);
        } else {
            writeBlocks(t.oldBuf,t.oldBufSize);
        }
    } catch ( std::exception& e ) {
        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            HexDump::string_type dump;
            const void* buf = takeNew ? t.newBuf : t.oldBuf;
            const size_t bufSize = takeNew ? t.newBufSize : t.oldBufSize;
            dump.reserve(hd.hexdumpsize(bufSize)+hd.strdumpsize(bufSize)+10);
            hd.hexdump(dump,buf,bufSize);
            hd.strdump(dump,buf,bufSize);
            smsc_log_debug(log_,"failed to apply %s: %s, buf=%s",
                           takeNew ? "new" : "old", t.toString().c_str(), hd.c_str(dump));
        }
        throw smsc::util::Exception("failed to apply %s #%u: %s",
                                    takeNew ? "new" : "old",
                                    t.getSerial(), e.what() );
    }
}


void BlocksHSStorage2::applyJournalState( const JournalRecord& rec, bool takeNew )
{
    const Transaction& t = static_cast<const Transaction&>(rec);
    const StorageState& state( takeNew ? t.newState : t.oldState );
    if ( state.fileCount < files_.size() ) {
        std::for_each(files_.begin()+state.fileCount, files_.end(), smsc::util::PtrDestroy() );
        files_.erase(files_.begin()+state.fileCount, files_.end());
    }
    freeChain_.clear();
    freeCount_ = state.freeCount;
    freeChain_.push_back( state.ffb );
    if ( ! readFreeBlocks(1) ) {
        throw smsc::util::Exception( "readFreeBlocks(%s,#%u) failed",
                                     takeNew ? "new" : "old", t.getSerial() );
    }
}

} // namespace storage
} // namespace util
} // namespace scag2
