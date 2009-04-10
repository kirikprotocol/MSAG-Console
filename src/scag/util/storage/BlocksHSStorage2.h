#ifndef _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE2_H
#define _SCAG_UTIL_STORAGE_BLOCKSHSSTORAGE2_H

#include "util/int.h"
#include "Serializer.h"
#include <cstring>

namespace scag2 {
namespace util {
namespace storage {

class BlocksHSStorage2
{
public:
    typedef uint64_t         index_type;
    typedef Serializer::Buf  buffer_type;

public:
    BlocksHSStorage2( size_t blockSize, unsigned extraSize = 0 ) :
    packer_(blockSize,0),
    headerSize_(navSize()+extraSize+idxSize())
    {
        assert(blockSize_ > navSize());
    }

    /// Modifies storage:
    ///
    /// the data with key 'key' (for logging only),
    /// which was stored starting at index 'oldIndex',
    /// having old datablock content in 'oldBuf'
    ///    is being replaced with
    /// should be replaced with a new content in 'newBuf'.
    ///
    /// Typical cases:
    /// 1. newBuf = 0 -- data should be removed;
    /// 2. oldIndex == invalid -- data should be appended (created).
    ///
    /// Return invalidIndex() on failure, the index of new data on success.
    index_type change( const char* key, index_type oldIndex,
                       buffer_type* oldBuf, buffer_type* newBuf ) {
        if ( !oldBuf ) {
            oldBuf = &changeBuf_;
            if ( !read(oldIndex,*oldBuf,newBuf ? (newBuf->size()-1)/ blockSize_+1 : 0) ) {
                return invalidIndex();
            }
        }
        // oldbuf now contains old transaction data

        if ( !newBuf ) {
            // data will be deleted, i.e. all blocks in oldBuf should be freed.
            newBuf = &freeBuf_;
            freeBlocks(*oldBuf,*newBuf);
        } else {
            // fill newBuf data with actual blocks indices
            linkBlocks(*oldBuf,*newBuf);
        }

        return applyChanges(*oldBuf,*newBuf);
    }

    /// Read data, starting from index 'index' to the buffer buf (as is, i.e. w/o unpacking).
    /// Return true, if the read was successful.
    bool read( index_type index, buffer_type& buf ) {
        buf->clear();
        return false;
    }

    
    /// should be performed only to the 
    void packBuffer( buffer_type& buffer ) const {
        size_t oldSize = buffer.size();
        size_t pos = blockSize_+idxSize();
        if ( oldSize <= pos ) return;
        const size_t totalNonHeadBlocks = ( oldSize - pos - 1 ) /
            ( blockSize_ - navSize() ) + 1;
        const size_t newSize = oldSize+totalNonHeadBlocks*navSize();
        buffer.resize(newSize);
        // copy all blocks except the last
        for ( size_t blk = 1; blk < totalNonHeadBlocks; ++blk ) {
            memcpy(&buffer[oldSize], &buffer[pos], navSize());
            oldSize += navSize();
            pos += blockSize_;
        }
        // copying the last block
        if ( oldSize < pos+navSize() ) {
            // overlap
            memcpy(&buffer[pos+navSize()],&buffer[pos],oldSize-pos);
        } else {
            memcpy(&buffer[oldSize],&buffer[pos],navSize());
        }
        oldSize += navSize();
        assert(oldSize == newSize);
    }


    void unpackBuffer( buffer_type& buffer ) const {
        size_t oldSize = buffer.size();
        if ( oldSize <= blockSize_ ) return;
        const size_t totalNonHeadBlocks = ( oldSize - 1 ) / blockSize_;
        size_t pos = totalNonHeadBlocks*blockSize_;
        // copying the last block
        oldSize -= navSize();
        if ( oldSize < pos+navSize() ) {
            // overlap
            memcpy(&buffer[pos],&buffer[pos+navSize()],oldSize-pos);
        } else {
            memcpy(&buffer[pos],&buffer[oldSize],navSize());
        }
        for ( size_t blk = totalNonHeadBlocks-1; blk > 0; --blk ) {
            oldSize -= navSize();
            pos -= blockSize_;
            memcpy(&buffer[pos], &buffer[oldSize], navSize());
        }
        buffer.resize(oldSize);
    }

    inline index_type invalidIndex() const {
        return BlockNavigation::invalidIndex();
    }

    inline size_t headerSize() const {
        return headerSize_;
    }


private:
    inline size_t navSize() const { return BlockNavigation::persistentSize(); }

    bool read( index_type oldIndex, buffer_type& oldBuf, size_t needBlocks )
    {
        oldBuf.resize(navSize());
        if ( oldIndex != invalidIndex() ) {
            size_t curPos = 0;
            size_t toRead = 0;
            File* f = getFile(oldIndex);
            if (!f) return false;

            // read next navigation
            off_t offset = getOffset(oldIndex);
            f->Seek(offset);
            f->Read(&oldBuf[curPos],navSize());

            // decode it
            BlockNavigation bn;
            {
                Deserializer dsr(oldBuf);
                ser.setrpos(curPos);
                bn.load(ser);
            }
            if (curPos == 0) {
                // should be head
                if (!bn.isHead()) return false;
                if (bn.dataSize() <= navSize()) return false;
                oldBuf.resize(bn.dataSize());
                toRead = bn.dataSize();
            } else {
                if (!bn.isUsed()) return false;
                if (bn.refBlock()!=oldIndex) return false;
            }
            curPos += navSize();
            toRead -= navSize();
            size_t toRead = std::min(oldBuf.size(),blockSize_)-curPos;
            f->Read(&oldBuf[curPos],std::min(oldBuf.size(),blockSize_));
        } while ( true );
    }
    /*

        // lets write some garbage to blocks
        if ( newBuf ) {
            size_t totalNonHeadBlocks = (newBuf->size()-1) / blockSize_;
            Serializer ser(*newBuf);
            BlockNavigation bn;
            bn.setNextBlock(invalidIndex());
            bn.setRefBlock(invalidIndex());
            for ( size_t blk = 1; blk <= totalNonHeadBlocks; ++blk ) {
                ser.setwpos(blk*blockSize_);
                bn.save(ser);
            }
        }
        return invalidIndex();
    }
     */

private:
    size_t blockSize_;
    size_t headerSize_;

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
