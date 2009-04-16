#ifndef _SCAG_UTIL_STORAGE_HSPACKER_H
#define _SCAG_UTIL_STORAGE_HSPACKER_H

#include "util/int.h"
#include "logger/Logger.h"
#include "Serializer.h"
#include "core/buffers/File.hpp"
#include "scag/util/HexDump.h"
#include <cstring>
#include <sstream>
#include <iterator>

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
    typedef uint64_t offset_type;

public:
    // 7 bytes
    inline static offset_type navBits() { return 0x00ffffffffffffffULL; }
    inline static offset_type badBit()  { return 0x0080000000000000ULL; }
    inline static size_t navSize() { return 16; }
    inline static size_t idxSize() { return 8; }

    inline void setNextBlock( offset_type next ) {
        next_ = next;
    }
    inline void setRefBlock( offset_type refBlock ) {
        used_ = true;
        head_ = false;
        pack_ = 0;
        ref_ = refBlock;
    }
    inline void setDataSize( offset_type dataSize, uint8_t packType ) {
        used_ = head_ = true;
        pack_ = packType;
        ref_ = dataSize;
    }
    inline void setFreeCells( offset_type freeCells ) {
        used_ = head_ = false;
        pack_ = 0;
        ref_ = freeCells;
    }


    /// used both for free/used block chains.
    inline offset_type nextBlock() const { return next_; }

    /// the index of the reference block.
    /// only valid when ( isFree() == false && isHead() == false ).
    inline offset_type refBlock() const { return ref_; }

    /// the length of data in the used chain.
    /// only valid when isHead() == true.
    inline offset_type dataSize() const { return ref_; }

    /// the number of free cells in the tail of the free cell chain.
    /// only valid when isFree() == true.
    inline offset_type freeCells() const { return ref_; }

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


    /// read from file
    inline void load( smsc::core::buffers::File& f ) {
        smsc::core::buffers::TmpBuf<unsigned char,32> buf(navSize());
        f.Read(buf.GetCurPtr(),navSize());
        Deserializer dsr(buf.GetCurPtr(),navSize());
        load(dsr);
    }

    inline void save( smsc::core::buffers::File& f ) const {
        smsc::core::buffers::TmpBuf<unsigned char,32> tmpbuf(navSize());
        save(tmpbuf.GetCurPtr());
        f.Write(tmpbuf.GetCurPtr(),navSize());
    }

    void save( Serializer& ser ) const {
        ser.reserve(navSize());
        size_t curpos = ser.wpos();
        ser << next_;
        ser << ref_;
        // setting state and pack
        unsigned char* p = ser.data() + curpos;
        const uint8_t state( (used_ ? USED_BIT : 0) | (head_ ? HEAD_BIT : 0) );
        *p = state;
        p += idxSize();
        *p = pack_;
    }

    void save( void* wher ) const {
        unsigned char* where = reinterpret_cast<unsigned char*>(wher);
        EndianConverter cvt;
        unsigned char* p = reinterpret_cast<unsigned char*>(cvt.set(next_));
        const uint8_t state( (used_ ? USED_BIT : 0) | (head_ ? HEAD_BIT : 0) );
        *p = state;
        memcpy(where, p, idxSize());
        p = reinterpret_cast<unsigned char*>(cvt.set(ref_));
        *p = pack_;
        memcpy(where+idxSize(),p,idxSize());
    }

protected:
    uint64_t next_;
    uint64_t ref_;
    uint8_t  pack_;
    bool     used_;
    bool     head_;
};




/// --------------------------------------------------
/// class for repacking HS buffer.
/// It has two coordinate systems:
///  1. indices of blocks in datafile, starting from 0
///  2. absolute offsets of blocks in datafile, starting from 0
/// Typically, all internals work with offsets.
/// Indices are used for interface to client code.
/// --------------------------------------------------
class HSPacker
{
public:
    typedef BlockNavigation::offset_type offset_type;
    typedef uint64_t                     index_type;
    typedef Serializer::Buf              buffer_type;

public:
    HSPacker( size_t blockSize, uint8_t packType, smsc::logger::Logger* logger = 0 ) :
    blockSize_(blockSize), packingType_(packType), log_(logger)
    {
        // assert(blockSize_ > 2*navSize());
        notUsed_ = blockSize_ + BlockNavigation::badBit();
        if ( blockSize_ <= 2*navSize() ) {
            throw smsc::util::Exception("block size = %u is too small, must be greater than 2*%u",
                                        unsigned(blockSize_), unsigned(navSize()) );
        }
    }

    inline size_t blockSize() const { return blockSize_; }

    inline uint8_t getPackingType() const { return packingType_; }
    
    /// method counts how many trailing blocks (non-head) is needed for buffer of given size
    inline size_t trailingBlocks( size_t unpackedSize ) const {
        if ( unpackedSize <= blockSize_ ) return 0;
        return ( unpackedSize - blockSize_ - 1 ) / ( blockSize_ - navSize() ) + 1;
    }


    /// method packs buffer according to packing scheme.
    /// @param buffer -- working buffer:
    ///   on input: serialized data started at idxSize()+navSize()+initialPosition
    ///   on output: serialized data is interleaved with trailing block headers,
    ///              buffer content before the data is not touched.
    /// @param headers -- pointer to the headers of trailing blocks;
    void packBuffer( buffer_type& buffer,
                     buffer_type* headers,
                     size_t initialPos = 0 ) const
    {
        if ( packingType_ != 0 ) {
            // only packtype 0 is implemented
            if (log_) { smsc_log_error(log_,"packing type = %u is not implemented", unsigned(packingType_) ); }
            ::abort();
        }
        assert(buffer.size() > initialPos);
        const size_t oldSize = buffer.size();
        assert( oldSize >= initialPos );
        const size_t trailBlocks = trailingBlocks( oldSize - initialPos );
        if ( !trailBlocks ) return;

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            std::string hex;
            hd.hexdump(hex,&buffer[initialPos],buffer.size()-initialPos);
            std::string hdx;
            if ( headers ) {
                hd.hexdump(hdx,
                           &((*headers)[headers->size()-trailBlocks*navSize()]),
                           trailBlocks*navSize());
            }
            smsc_log_debug(log_,"packBuffer buf: %s hdr: %s", hex.c_str(), hdx.c_str() );
        }

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

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            std::string hex;
            hd.hexdump(hex,&buffer[initialPos],buffer.size()-initialPos);
            smsc_log_debug(log_,"after pack: %s", hex.c_str() );
        }
    }


    void mergeHeaders( buffer_type& buffer,
                       const buffer_type& headers,
                       size_t initialPos = 0 )
    {
        assert( headers.size() >= idxSize() + navSize() );
        assert( buffer.size() > initialPos + idxSize() + navSize() );
        assert( countBlocks(buffer.size()-initialPos) == (headers.size()-idxSize())/navSize() );

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            std::string hex;
            hd.hexdump(hex,&buffer[initialPos],buffer.size()-initialPos);
            std::string hdx;
            hd.hexdump(hdx,&headers[0],headers.size());
            smsc_log_debug(log_,"mergeHeaders buf: %s hdr: %s", hex.c_str(), hdx.c_str() );
        }

        std::copy( headers.begin(), headers.begin()+idxSize(), buffer.begin() );
        size_t outpos = idxSize()+initialPos;
        for ( size_t pos = idxSize(); pos < headers.size(); pos += navSize() ) {
            std::copy( headers.begin()+pos, headers.begin()+pos+navSize(), buffer.begin()+outpos );
            outpos += blockSize();
        }

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            std::string hex;
            hd.hexdump(hex,&buffer[initialPos],buffer.size()-initialPos);
            smsc_log_debug(log_,"after merge: %s", hex.c_str() );
        }
    }


    /// method unpack the buffer interleaved with block headers.
    /// @param buffer -- on output: serialized data prepended with idxSize()
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

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            std::string hex;
            hd.hexdump(hex,&buffer[initialPos],buffer.size()-initialPos);
            smsc_log_debug(log_,"unpackBuffer buf: %s", hex.c_str());
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
            // fprintf(stderr,"blk:%u topos:%u frompos:%u\n", blk, topos, frompos );
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

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            std::string hex;
            hd.hexdump(hex,&buffer[initialPos],buffer.size()-initialPos);
            std::string hdx;
            if ( headers ) {
                hd.hexdump(hdx,&((*headers)[headers->size()-trailBlocks*navSize()]),trailBlocks*navSize());
            }
            smsc_log_debug(log_,"after unpack: buf: %s hdr: %s", hex.c_str(), hdx.c_str() );
        }
    }


    /// extract block information from block buffer.
    /// buffer must be in form: idx nav data nav data ... [ free-idx free-nav free-nav ... ]
    /// blocks on output will contain:
    /// idx datasize idx datasize ... corresponding to the buffer contents.
    /// @return the offset of the last next_block field in the chain.
    offset_type extractBlocks( const buffer_type& buffer,
                               std::vector<offset_type>& blocks,
                               offset_type prevffb,
                               size_t initialPosition = 0 )
    {
        blocks.clear();
        Deserializer dsr(buffer);
        dsr.setrpos(initialPosition);
        bool isUsed = true;
        offset_type nextBlock = notUsed();

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            std::string hex;
            hd.hexdump(hex,&buffer[initialPosition],buffer.size()-initialPosition);
            smsc_log_debug(log_,"extractBlocks buf: %s", hex.c_str());
        }

        offset_type headBlock = notUsed();
        do {
            // write the empty block
            size_t dataSize = 0;
            blocks.push_back(nextBlock);
            blocks.push_back(offset_type(dataSize));

            dsr >> nextBlock;

            while ( nextBlock != notUsed() ) {

                BlockNavigation bn;
                {
                    const size_t curpos = dsr.rpos();
                    bn.load(dsr);
                    dsr.setrpos(curpos);
                }

                offset_type idx;

                if ( bn.isFree() ) {

                    if ( isUsed ) {
                        // add a check for used chain
                        // ::abort();
                        isUsed = false;
                    }
                    dataSize = navSize();

                } else {

                    if ( !isUsed ) {
                        // only one isused chain is allowed
                        if (log_) {
                            smsc_log_error(log_,"only one is used chain allowed");
                        }
                        ::abort();
                    }

                    if ( bn.isHead() ) {
                        if ( dataSize > 0 ) {
                            if (log_) {
                                smsc_log_error(log_,"head is found while dataSize > 0");
                            }
                            ::abort();
                        }
                        dataSize = bn.dataSize();
                        headBlock = nextBlock;
                    } else {
                        // used
                        if (dataSize == 0) {
                            if (log_) {
                                smsc_log_error(log_,"used is found while dataSize == 0");
                            }
                            ::abort();
                        }
                        if (bn.refBlock() != headBlock) {
                            if (log_) {
                                smsc_log_error(log_,"head_block reference is %llx, should be %llx",
                                               bn.refBlock(), headBlock);
                                ::abort();
                            }
                        }
                    }
                }

                idx = bn.nextBlock();
                const size_t toWrite = std::min(blockSize(),dataSize);
                if ( dsr.size() < dsr.rpos() + toWrite ) {
                    if (log_) {
                        smsc_log_error(log_,"too few data in block chain");
                    }
                    ::abort();
                }

                blocks.push_back(nextBlock);
                blocks.push_back(toWrite);

                dsr.setrpos(dsr.rpos()+toWrite);
                dataSize -= toWrite;
                nextBlock = idx;
                if ( dsr.rpos() >= dsr.size() ) { break; }
            }
            
            if ( dataSize > 0 ) {
                // too few blocks
                if (log_) {
                    smsc_log_error(log_,"too few blocks found");
                }
                ::abort();
            }

            if ( dsr.rpos() >= dsr.size() ) { break; }
            isUsed = false;

        } while ( true );

        if (log_ && log_->isDebugEnabled()) {
            std::ostringstream os;
            os.setf( std::ios::hex, std::ios::basefield );
            std::copy( blocks.begin(), blocks.end(),
                       std::ostream_iterator<offset_type>(os," ") );
            smsc_log_debug( log_,"blocks extracted: %s, next: %llx",
                            os.str().c_str(), nextBlock );
        }

        /// if free chain has not started and used chain already finished,
        /// then prevffb is not changed, otherwise prevffb should be replaced with nextBlock
        return ( isUsed && nextBlock == notUsed() ) ? prevffb : nextBlock;
    }


    /// on return offsets is filled with offsets of affected blocks.
    void extractOffsets( const std::vector< offset_type >& blocks,
                         std::vector< offset_type >& offsets )
    {
        assert( blocks.size() % 2 == 0 );

        offsets.clear();
        offsets.reserve(blocks.size()/2+2);
        for ( size_t pos = 0; pos < blocks.size(); ++pos ) {
            const offset_type i = blocks[pos];
            const size_t sz = blocks[++pos];
            if (i == notUsed() || sz == 0) continue;
            offsets.push_back(i);
        }
    }


    /// method prepares headers for used chain of length dataSize.
    void makeHeaders( buffer_type&              headers,
                      std::vector<offset_type>& offsets,
                      size_t                    dataSize )
    {
        // offsets contains affected block indices
        const size_t needBlocks = countBlocks( dataSize+idxSize() );
        if ( needBlocks > offsets.size() ) {
            if (log_) {smsc_log_error(log_,"too few offsets in vector");}
            ::abort();
        }

        headers.clear();
        headers.reserve(needBlocks*navSize()+idxSize());

        Serializer ser(headers);
        std::vector< offset_type >::const_iterator iter = offsets.begin();
        const offset_type firstBlock = *iter++;
        ser << firstBlock;
        BlockNavigation bn;
        bn.setDataSize( dataSize, packingType_ );
        for ( size_t i = 0; i < needBlocks; ++i ) {

            if ( dataSize == 0 ) {
                // too many blocks
                if (log_) {smsc_log_error(log_,"too many blocks counted");}
                ::abort();
            }

            if ( i+1 == needBlocks ) {
                bn.setNextBlock(notUsed());
            } else {
                bn.setNextBlock(*iter++);
            }
            bn.save(ser);

            dataSize -= ( dataSize >= blockSize() ? blockSize() : dataSize );
            bn.setRefBlock(firstBlock);

        }
        if ( dataSize > 0 ) {
            // too few blocks
            if (log_) {smsc_log_error(log_,"too few blocks");}
            ::abort();
        }

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            std::string hex;
            if (headers.size()>0) { hd.hexdump(hex,&headers[0],headers.size()); }
            smsc_log_debug( log_,"headers made: %s", hex.c_str());
        }
    }


    offset_type makeFreeChain( buffer_type& profile,
                               std::vector< offset_type >& offsets,
                               size_t startPos,
                               offset_type prevffb )
    {
        assert( offsets.size() > startPos );
        const size_t oldSize = profile.size();
        profile.reserve( oldSize + (offsets.size() - startPos)*navSize()+idxSize() );
        BlockNavigation bn;
        bn.setFreeCells( notUsed() );
        Serializer ser(profile);
        ser.setwpos(profile.size());
        std::vector< offset_type >::const_iterator iter = offsets.begin() + startPos;
        const offset_type newffb = *iter++;
        ser << newffb;
        for ( size_t i = startPos; i < offsets.size(); ++i ) {
            if ( i+1 == offsets.size() ) {
                bn.setNextBlock(prevffb);
            } else {
                bn.setNextBlock(*iter++);
            }
            bn.save(ser);
        }
        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            std::string hex;
            hd.hexdump(hex,&profile[oldSize],profile.size()-oldSize);
            smsc_log_debug( log_,"free chain made: %s", hex.c_str());
        }
        return newffb;
    }

    /// index value corresponding to non-valid index.
    inline static index_type invalidIndex() {
        return 0xffffffffffffffffULL;
    }

    /// offset value representing not used item.
    inline offset_type notUsed() const { return notUsed_; }

    inline bool isNotUsed( offset_type o ) const {
        return bool(o & BlockNavigation::badBit());
    }

    inline size_t navSize() const { return BlockNavigation::navSize(); }

    inline size_t idxSize() const { return BlockNavigation::idxSize(); }

    /// what is the needed amount of blocks?
    inline size_t countBlocks( size_t packedSize ) const {
        if ( packedSize <= idxSize() ) return 0;
        return (packedSize - idxSize() - 1) / blockSize() + 1;
    }

    /// block index to offset
    inline offset_type idx2pos( index_type idx ) const {
        if ( idx == invalidIndex() ) return notUsed_;
        return offset_type(idx*blockSize_);
    }

    inline index_type pos2idx( offset_type pos ) const {
        if ( (pos & BlockNavigation::badBit()) ) return invalidIndex();
        if ( pos % blockSize_ != 0 ) {
            if (log_) {
                smsc_log_error( log_,"offset %llx is not divisable by blocksize=%x",
                                pos, unsigned(blockSize_) );
            }
            ::abort();
        }
        // assert( (pos % blockSize_) == 0 );
        return index_type(pos / blockSize_);
    }

private:
    size_t                blockSize_;
    uint8_t               packingType_;
    offset_type           notUsed_;
    smsc::logger::Logger* log_;
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
