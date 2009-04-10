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
        return ( unpackedSize - blockSize_ - 1 ) / ( blockSize_ - navSize() ) + 1;
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
        const size_t oldSize = buffer.size();
        assert( oldSize >= initialPos );
        const size_t trailBlocks = trailingBlocks( oldSize - initialPos );
        if ( !trailBlocks ) return;
        size_t headerPos;
        if ( headers ) {
            assert(headers->size() >= trailBlocks*navSize());
            headerPos = headers->size() - trailBlocks*navSize(); // last trailblocks
        }
        const size_t newSize = oldSize + trailBlocks*navSize();
        buffer.resize(newSize);
        size_t frompos = initialPos + blockSize_;
        size_t topos = oldSize;
        for ( size_t blk = 1; blk <= trailBlocks; ++blk ) {
            fprintf(stderr,"blk:%u topos:%u frompos:%u\n", blk, topos, frompos );
            if ( blk == trailBlocks && topos < frompos+navSize() ) {
                // overlap detected in last block
                assert(topos>frompos);
                const size_t trail = topos-frompos;
                memcpy(&buffer[frompos+navSize()],&buffer[frompos],trail);
            } else {
                memcpy(&buffer[topos],&buffer[frompos],navSize());
            }
            if ( headers ) {
                // restore headers
                memcpy(&buffer[frompos],&((*headers)[headerPos]), navSize());
                headerPos += navSize();
            }
            topos += navSize();
            frompos += blockSize_;
        }
    }


    void unpackBuffer( buffer_type& buffer,
                       buffer_type* headers,
                       size_t initialPos = 0 ) const
    {
        if ( packingType_ != 0 ) {
            ::abort();
        }
        const size_t oldSize = buffer.size();
        assert( oldSize >= initialPos );
        if ( oldSize <= initialPos+blockSize_ ) return;
        const size_t trailBlocks = (oldSize - initialPos - 1) / blockSize_;
        size_t headerPos;
        if ( headers ) {
            headers->resize( headers->size()+trailBlocks*navSize() );
            headerPos = headers->size();
        }
        size_t frompos = initialPos + trailBlocks*blockSize_; // points to the last block
        size_t topos = oldSize; // points past buffer
        for ( size_t blk = trailBlocks; blk > 0; --blk ) {
            if ( headers ) {
                // save headers
                headerPos -= navSize();
                memcpy(&((*headers)[headerPos]),&buffer[frompos],navSize());
            }
            topos -= navSize();
            fprintf(stderr,"blk:%u topos:%u frompos:%u\n", blk, topos, frompos );
            if ( blk == trailBlocks && topos < frompos+navSize() ) {
                // overlap detected in last block
                assert(topos>frompos);
                const size_t trail = topos-frompos;
                memcpy(&buffer[frompos],&buffer[frompos+navSize()],trail);
            } else {
                memcpy(&buffer[frompos],&buffer[topos],navSize());
            }
            frompos -= blockSize_;
        }
        buffer.resize(topos);
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
