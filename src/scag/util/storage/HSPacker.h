#ifndef _SCAG_UTIL_STORAGE_HSPACKER_H
#define _SCAG_UTIL_STORAGE_HSPACKER_H

#include "util/int.h"
#include "Serializer.h"
#include <cstring>

namespace scag2 {
namespace util {
namespace storage {

/// class which represents a navigation header of HS datablock.
/// format:
///               6666555555555544444444443333333333222222222211111111110000000000
///               3210987654321098765432109876543210987654321098765432109876543210
///
/// 0th qword:    UxxxxxxHLnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
///  U         -- used bit: 1 -- block is used, 0 -- block is free
///  H         -- head bit (only when U==1): 1 -- block is the head, 0 -- not the head
///  xxxxxx    -- reserved
///  L         -- last block: 1 -- block is the last in the chain, 0 -- block is not the last
///  nn...nn   -- next_block_offset (when L==0), or blockSize (when L==1)
///
/// 1st qword:    ppppppppXsssssssssssssssssssssssssssssssssssssssssssssssssssssss
///  pppppppp  -- pack_type (when H==1), or reserved (when H==0)
///  X         -- reserved
///  sssssss   -- data_size (when H==1), or free_blocks_count (when U==0),
///               or head_block_offset (when U==1,H==0)
///  
class BlockNavigation
{
private:
    enum {
            USED_BIT = 0x80,
            HEAD_BIT = 0x01
    };

public:
    typedef uint64_t index_type;

public:
    // 7 bytes
    inline static index_type navBits() { return 0x00ffffffffffffffULL; }
    inline static index_type badBit()  { return 0x0080000000000000ULL; }
    inline static size_t navSize() { return 16; }
    inline static size_t idxSize() { return 8; }

    inline void setNextBlock( index_type next ) {
        next_ = next;
    }
    inline void setRefBlock( index_type refBlock ) {
        used_ = true;
        head_ = false;
        pack_ = 0;
        ref_ = refBlock;
    }
    inline void setDataSize( index_type dataSize, uint8_t packType ) {
        used_ = head_ = true;
        pack_ = packType;
        ref_ = dataSize;
    }
    inline void setFreeCells( index_type freeCells ) {
        used_ = head_ = false;
        pack_ = 0;
        ref_ = freeCells;
    }


    /// used both for free/used block chains.
    inline index_type nextBlock() const { return next_; }

    /// the index of the reference block.
    /// only valid when ( isFree() == false && isHead() == false ).
    inline index_type refBlock() const { return ref_; }

    /// the length of data in the used chain.
    /// only valid when isHead() == true.
    inline index_type dataSize() const { return ref_; }

    /// the number of free cells in the tail of the free cell chain.
    /// only valid when isFree() == true.
    inline index_type freeCells() const { return ref_; }

    /// the packing type
    /// only valid when isHead() == true
    inline uint8_t packingType() const { return pack_; }

    /// if the block is free.
    inline bool isFree() const { return !used_; }

    /// NOTE: post-condition isHead() == true => isFree() === false.
    inline bool isHead() const { return head_; }

    inline void load( Deserializer& dsr ) {
        const uint8_t state = *dsr.curpos();
        used_ = (state & USED_BIT);
        head_ = (state & HEAD_BIT);
        uint64_t val;
        dsr >> val;
        next_ = (val & navBits());
        pack_ = *dsr.curpos();
        dsr >> val;
        ref_ = (val & navBits());
    }

    void save( Serializer& ser ) const {
        ser.reserve( navSize() );
        size_t curpos = ser.wpos();
        ser << next_;
        ser << ref_;
        // setting state and pack
        unsigned char* p = const_cast<unsigned char*>(ser.data()+curpos);
        const uint8_t state( (used_ ? USED_BIT : 0) | (head_ ? HEAD_BIT : 0) );
        *p = state;
        p += idxSize();
        *p = pack_;
    }

protected:
    uint64_t next_;
    uint64_t ref_;
    uint8_t  pack_;
    bool     used_;
    bool     head_;
};


/// class for repacking HS buffer.
class HSPacker
{
public:
    typedef BlockNavigation::index_type  index_type;
    typedef Serializer::Buf              buffer_type;

public:
    HSPacker( size_t blockSize, uint8_t packType ) :
    blockSize_(blockSize), packingType_(packType)
    {
        assert(blockSize_ > 2*navSize());
        invalid_ = blockSize_ + BlockNavigation::badBit();
    }

    inline uint8_t blockSize() const { return blockSize_; }

    inline uint8_t getPackingType() const { return packingType_; }
    
    /// method counts how many trailing blocks (non-head) is needed for buffer of given size
    inline size_t trailingBlocks( size_t unpackedSize ) const {
        if ( unpackedSize <= blockSize_ ) return 0;
        return ( unpackedSize - 1 ) / ( blockSize_ - navSize() );
    }


    /// method packs buffer according to different packing scheme.
    void packBuffer( buffer_type& buffer,
                     buffer_type* headers,
                     size_t initialPos = 0 ) const
    {
        if ( packingType_ != 0 ) {
            // only packtype 0 is implemented
            ::abort();
        }
        size_t oldSize = buffer.size();
        assert( oldSize >= initialPos );
        const size_t trailBlocks = trailingBlocks( oldSize - initialPos );
        if ( !trailBlocks ) return;
        size_t pos = initialPos + blockSize_;
        size_t headerPos;
        if ( headers ) {
            assert(headers->size() >= trailBlocks*navSize());
            // the last trailBlocks are used
            headerPos = headers->size() - trailBlocks*navSize();
        }
        const size_t newSize = oldSize+trailBlocks*navSize();
        buffer.resize(newSize);
        // copy all blocks except the last
        for ( size_t blk = 1; blk <= trailBlocks; ++blk ) {
            if ( oldSize < pos+navSize() ) {
                // overlap detected
                if ( oldSize < pos ) {
                    // first part is copyed before the pos
                    const size_t trail = pos-oldSize;
                    memcpy(&buffer[oldSize],&buffer[pos],trail);
                    memcpy(&buffer[pos+navSize()],&buffer[pos+trail],navSize()-trail);
                    oldSize += navSize(); // extra move
                } else {
                    memcpy(&buffer[pos+navSize()],&buffer[pos],oldSize-pos);
                }
            } else {
                memcpy(&buffer[oldSize], &buffer[pos], navSize());
            }
            if ( headers ) {
                // restore headers
                memcpy(&buffer[pos],&((*headers)[headerPos]), navSize());
                headerPos += navSize();
            }
            oldSize += navSize();
            pos += blockSize_;
        }
        assert(oldSize == newSize);
    }


    void unpackBuffer( buffer_type& buffer,
                       buffer_type* headers,
                       size_t initialPos = 0 ) const
    {
        if ( packingType_ != 0 ) {
            ::abort();
        }
        size_t oldSize = buffer.size();
        assert( oldSize >= initialPos );
        size_t pos = initialPos;
        if ( oldSize <= pos+blockSize_ ) return;
        const size_t trailBlocks = ( oldSize - pos - 1 ) / blockSize_;
        pos += trailBlocks*blockSize_; // points to the head of the last block
        size_t headerPos;
        if ( headers ) {
            headers->resize( headers->size()+trailBlocks*navSize() );
            headerPos = headers->size();
        }
        for ( size_t blk = trailBlocks; blk > 0; --blk ) {
            oldSize -= navSize();
            if ( headers ) {
                headerPos -= navSize();
                memcpy( &(*headers)[headerPos],&buffer[pos],navSize());
            }
            if ( oldSize < pos+navSize() ) {
                // last block overlap
                memcpy(&buffer[pos],&buffer[pos+navSize()],oldSize-pos);
            } else {
                memcpy(&buffer[pos], &buffer[oldSize], navSize());
            }
            pos -= blockSize_;
        }
        buffer.resize(oldSize);
    }

    inline index_type invalidIndex() const {
        return 0xffffffffffffffffULL;
    }

    inline size_t navSize() const { return BlockNavigation::navSize(); }

    inline size_t idxSize() const { return BlockNavigation::idxSize(); }

    /// block index to offset
    inline index_type idx2pos( index_type idx ) const {
        if ( idx == invalidIndex() ) return invalid_;
        return idx*blockSize_;
    }

    inline index_type pos2idx( index_type pos ) {
        if ( (pos & BlockNavigation::badBit()) ) return invalidIndex();
        assert( (pos % blockSize_) == 0 );
        return pos / blockSize_;
    }

private:
    size_t blockSize_;
    size_t packingType_;
    size_t invalid_;
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

#endif /* ! _SCAG_UTIL_STORAGE_HSPACKER_H */
