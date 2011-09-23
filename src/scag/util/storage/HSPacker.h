#ifndef _SCAG_UTIL_STORAGE_HSPACKER_H
#define _SCAG_UTIL_STORAGE_HSPACKER_H

#include "util/int.h"
#include "logger/Logger.h"
#include "scag/util/io/Serializer.h"
#include "core/buffers/File.hpp"
#include "scag/util/io/HexDump.h"
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
public:
    enum {
            NAV_SIZE = 16 // may be used in dependent code to e.g. create char arrays
    };
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
    inline static size_t navSize() { return NAV_SIZE; }
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

    /// helper: load index from memory
    static inline offset_type loadIdx( const void* wher ) {
        return (io::EndianConverter::get64(wher) & navBits());
    }

    static inline void saveIdx( void* wher, offset_type idx ) {
        io::EndianConverter::set64(wher,idx & navBits());
    }

    /// load from a block of memory, considering it has been filled
    void loadPtr( const void* wher ) {
        register const uint8_t* where = reinterpret_cast<const uint8_t*>(wher);
        const uint8_t state = *where;
        used_ = state & USED_BIT;
        head_ = state & HEAD_BIT;
        next_ = io::EndianConverter::get64(where) & navBits();
        where += idxSize();
        pack_ = *where;
        ref_ = io::EndianConverter::get64(where) & navBits();
    }

    /// save to a block of memory, which should be at least NAV_SIZE in length
    void savePtr( void* wher ) const {
        register uint8_t* where = reinterpret_cast<uint8_t*>(wher);
        io::EndianConverter::set64(where,next_);
        *where = (used_ ? USED_BIT : 0) | (head_ ? HEAD_BIT : 0);
        where += idxSize();
        io::EndianConverter::set64(where,ref_);
        *where = pack_;
    }

    // helper methods to load from different sources
    inline void load( io::Deserializer& dsr ) {
        const uint8_t* cp = dsr.curpos();
        dsr.setrpos(dsr.rpos()+navSize()); // check that it has enough data
        loadPtr(cp);
    }

    // save to serializer
    void save( io::Serializer& ser ) const {
        register size_t wpos = ser.wpos();
        ser.setwpos(wpos+navSize());
        savePtr(ser.data()+wpos);
    }

    // read from file
    inline void load( smsc::core::buffers::File& f ) {
        char buf[NAV_SIZE];
        f.Read(buf,navSize());
        loadPtr(buf);
    }

    // save to file
    inline void save( smsc::core::buffers::File& f ) const {
        char buf[NAV_SIZE];
        savePtr(buf);
        f.Write(buf,navSize());
    }

    std::string toString() const {
        char buf[80];
        if ( isFree() ) {
            snprintf(buf,sizeof(buf),"free next=%llx tail=%lld",
                     nextBlock(), freeCells() );
        } else if ( isHead() ) {
            snprintf(buf,sizeof(buf),"head next=%llx size=%lld",
                     nextBlock(), dataSize() );
        } else {
            snprintf(buf,sizeof(buf),"used next=%llx refb=%llx",
                     nextBlock(), refBlock() );
        }
        return buf;
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
    typedef io::Serializer::Buf          buffer_type;

public:
    HSPacker( size_t blockSize, uint8_t packType, smsc::logger::Logger* logger = 0 ) :
    blockSize_(blockSize), packingType_(packType), log_(logger)
    {
        notUsed_ = blockSize_ + BlockNavigation::badBit();
        if ( blockSize_ <= 2*navSize() ) {
            throw smsc::util::Exception("block size = %u is too small, must be greater than 2*%u",
                                        unsigned(blockSize_), unsigned(navSize()) );
        }
    }

    inline size_t blockSize() const { return blockSize_; }

    inline uint8_t getPackingType() const { return packingType_; }
    
    /// method counts how many trailing blocks (non-head) is needed for buffer of given size
    inline size_t trailingBlocks( size_t dataSize ) const {
        if ( dataSize <= blockSize_ ) return 0;
        return ( dataSize-blockSize_-1 ) / ( blockSize_ - navSize() ) + 1;
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
            if (log_) { smsc_log_fatal(log_,"packing type = %u is not implemented", unsigned(packingType_) ); }
            ::abort();
        }
        const size_t oldSize = buffer.size();
        if ( oldSize <= initialPos+idxSize()+blockSize() ) return;
        const size_t trailBlocks = trailingBlocks( oldSize-initialPos-idxSize() );
        if ( !trailBlocks ) return;

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            HexDump::string_type hex;
            hd.hexdump(hex,&buffer[initialPos],buffer.size()-initialPos);
            HexDump::string_type hdx;
            if ( headers ) {
                hd.hexdump(hdx,
                           &((*headers)[headers->size()-trailBlocks*navSize()]),
                           trailBlocks*navSize());
            }
            smsc_log_debug(log_,"packBuffer buf: sz=%u %s hdr: %s",
                           unsigned(buffer.size()-initialPos),
                           hd.c_str(hex), hd.c_str(hdx) );
        }

        buffer.resize(oldSize+trailBlocks*navSize());

        // size_t headerPos;
        buffer_type::iterator hptr;
        if ( headers ) {
            assert(headers->size() == (trailBlocks+1)*navSize()+idxSize());
            hptr = headers->begin();
            std::copy( hptr, hptr+idxSize()+navSize(), buffer.begin()+initialPos);
            hptr = headers->begin()+idxSize()+navSize();
        }
        // const size_t newSize = oldSize + idxSize() + trailBlocks*navSize();
        buffer_type::iterator iptr = buffer.begin() + initialPos + idxSize() + blockSize_;
        buffer_type::iterator optr = buffer.begin() + oldSize;
        for ( size_t blk = 1; blk <= trailBlocks; ++blk ) {
            if ( blk == trailBlocks && optr < iptr + navSize() ) {
                // overlap detected in last block
                assert( optr > iptr );
                // const size_t trail = optr-iptr;
                std::copy(iptr,optr,iptr+navSize());
                // memcpy(&buffer[frompos+navSize()],&buffer[frompos],trail);
            } else {
                std::copy(iptr,iptr+navSize(),optr);
                // memcpy(&buffer[topos],&buffer[frompos],navSize());
            }
            if ( headers ) {
                // restore headers
                std::copy(hptr,hptr+navSize(),iptr);
                // memcpy(&buffer[frompos],&((*headers)[headerPos]), navSize());
                hptr += navSize();
            }
            optr += navSize();
            iptr += blockSize();
        }

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            HexDump::string_type hex;
            hd.hexdump(hex,&buffer[initialPos],buffer.size()-initialPos);
            smsc_log_debug(log_,"after pack: sz=%u %s",
                           unsigned(buffer.size()-initialPos),
                           hd.c_str(hex) );
        }
    }


    void mergeHeaders( buffer_type& buffer,
                       const buffer_type& headers,
                       size_t initialPos = 0 )
    {
        assert( headers.size() >= idxSize() + navSize() );
        assert( buffer.size() > initialPos + idxSize() + navSize() );
        assert( countBlocks(buffer.size()-initialPos-idxSize()) == (headers.size()-idxSize())/navSize() );

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            HexDump::string_type hex;
            hd.hexdump(hex,&buffer[initialPos],buffer.size()-initialPos);
            HexDump::string_type hdx;
            hd.hexdump(hdx,&headers[0],headers.size());
            smsc_log_debug(log_,"mergeHeaders buf: sz=%u %s hdr: %s",
                           unsigned(buffer.size()-initialPos),
                           hd.c_str(hex), hd.c_str(hdx) );
        }

        {
            buffer_type::iterator optr = buffer.begin() + initialPos;
            buffer_type::const_iterator iptr = headers.begin();
            optr = std::copy( iptr, iptr+idxSize(), optr );
            iptr += idxSize();
            if ( iptr != headers.end() ) {
                while (true) {
                    std::copy( iptr, iptr+navSize(), optr );
                    iptr += navSize();
                    if ( iptr == headers.end() ) { break; }
                    optr += blockSize();
                }
            }
        }

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            HexDump::string_type hex;
            hd.hexdump(hex,&buffer[initialPos],buffer.size()-initialPos);
            smsc_log_debug(log_,"after merge: sz=%u %s",
                           unsigned(buffer.size()-initialPos),
                           hd.c_str(hex) );
        }
    }


    /// method unpack the buffer interleaved with block headers.
    /// @param buffer -- on output: serialized data prepended with idxSize()
    void unpackBuffer( buffer_type& buffer,
                       buffer_type* headers,
                       size_t initialPos = 0 ) const
    {
        if ( packingType_ != 0 ) {
            if (log_) { smsc_log_fatal(log_,"packing type %u is not implemented",unsigned(packingType_));}
            ::abort();
        }
        const size_t oldSize = buffer.size();
        if ( oldSize <= initialPos+idxSize()+navSize() ) return;
        const size_t trailBlocks = (oldSize-initialPos-idxSize()-1) / blockSize_;
        buffer_type::iterator hptr;
        if ( headers ) {
            headers->resize(idxSize()+(trailBlocks+1)*navSize() );
            std::copy(buffer.begin()+initialPos,
                      buffer.begin()+initialPos+idxSize()+navSize(),
                      headers->begin());
            hptr = headers->end();
        }
        if ( oldSize <= initialPos+idxSize()+blockSize() ) return;

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            HexDump::string_type hex;
            hd.hexdump(hex,&buffer[initialPos],buffer.size()-initialPos);
            smsc_log_debug(log_,"unpackBuffer buf: sz=%u %s", 
                           unsigned(buffer.size()-initialPos),
                           hd.c_str(hex));
        }

        buffer_type::iterator iptr = buffer.begin()+initialPos+idxSize()+trailBlocks*blockSize();
        buffer_type::iterator optr = buffer.begin()+oldSize;
        for ( size_t blk = trailBlocks; blk > 0; --blk ) {
            if ( headers ) {
                // save headers
                // headerPos -= navSize();
                // memcpy(&((*headers)[headerPos]),&buffer[frompos],navSize());
                hptr -= navSize();
                std::copy( iptr, iptr+navSize(), hptr );
            }
            optr -= navSize();
            // fprintf(stderr,"blk:%u topos:%u frompos:%u\n", blk, topos, frompos );
            if ( blk == trailBlocks && optr < iptr+navSize() ) {
                // overlap detected in last block
                assert(optr>iptr);
                // memcpy(&buffer[frompos],&buffer[frompos+navSize()],trail);
                std::copy(iptr+navSize(),optr+navSize(),iptr);
            } else {
                // memcpy(&buffer[frompos],&buffer[topos],navSize());
                std::copy(optr,optr+navSize(),iptr);
            }
            // frompos -= blockSize_;
            iptr -= blockSize();
        }
        buffer.erase(optr,buffer.end());

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            HexDump::string_type hex;
            hd.hexdump(hex,&buffer[initialPos],buffer.size()-initialPos);
            HexDump::string_type hdx;
            if ( headers ) {
                hd.hexdump(hdx,&((*headers)[headers->size()-trailBlocks*navSize()]),trailBlocks*navSize());
            }
            smsc_log_debug(log_,"after unpack: buf: sz=%u %s hdr: %s",
                           unsigned(buffer.size()-initialPos),
                           hd.c_str(hex), hd.c_str(hdx) );
        }
    }


    offset_type extractBlocks( const buffer_type& buffer,
                               std::vector< offset_type >& blocks,
                               offset_type prevffb,
                               size_t initialPosition = 0 )
    {
        return extractBlocks(&buffer[0],buffer.size(),blocks,prevffb,initialPosition);
    }


    /// extract block information from block buffer.
    /// buffer must be in form: idx nav data nav data ... [ free-idx free-nav free-nav ... ]
    /// blocks on output will contain:
    /// idx datasize idx datasize ... corresponding to the buffer contents.
    /// @return the offset of the last next_block field in the chain.
    offset_type extractBlocks( const void* buffer,
                               size_t      bufferSize,
                               std::vector<offset_type>& blocks,
                               offset_type prevffb,
                               size_t initialPosition = 0 )
    {
        blocks.clear();
        io::Deserializer dsr(buffer,bufferSize);
        dsr.setrpos(initialPosition);
        bool isUsed = true;
        offset_type nextBlock = notUsed();

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            HexDump::string_type hex;
            hd.hexdump(hex,buffer,bufferSize-initialPosition);
            smsc_log_debug(log_,"extractBlocks buf: %s", hd.c_str(hex));
        }

        offset_type headBlock = notUsed();
        do {
            // write the empty block
            size_t dataSize = 0;
            blocks.push_back(nextBlock);
            blocks.push_back(offset_type(dataSize));

            dsr >> nextBlock;

            while ( nextBlock != notUsed() ) {

                if ( nextBlock % blockSize() != 0 ) {
                    throw smsc::util::Exception("extracted block %llx is not at block boundary", nextBlock );
                }

                BlockNavigation bn;
                {
                    const size_t curpos = dsr.rpos();
                    bn.load(dsr);
                    dsr.setrpos(curpos);
                }

                offset_type idx;

                if ( bn.isFree() ) {

                    isUsed = false;
                    dataSize = navSize();

                } else {

                    if ( !isUsed ) {
                        // only one isused chain is allowed
                        throw smsc::util::Exception("broken chain (second used) at %llx",headBlock);
                    }

                    if ( bn.isHead() ) {
                        if ( dataSize > 0 ) {
                            throw smsc::util::Exception("broken chain (second head) at %llx", headBlock);
                        }
                        dataSize = bn.dataSize();
                        headBlock = nextBlock;
                    } else {
                        // used
                        if (dataSize == 0) {
                            throw smsc::util::Exception("broken chain (too many blocks) at %llx", headBlock );
                        }
                        if (bn.refBlock() != headBlock) {
                            throw smsc::util::Exception("broken chain (ref=%llx is wrong) at %llx", bn.refBlock(), headBlock);
                        }
                    }
                }

                idx = bn.nextBlock();
                const size_t toWrite = std::min(blockSize(),dataSize);
                if ( dsr.size() < dsr.rpos() + toWrite ) {
                    throw smsc::util::Exception("broken chain (buffer underrun) at head=%llx, cur=%s",
                                                headBlock, bn.toString().c_str() );
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
                throw smsc::util::Exception("broken chain (too few blocks) at %llx", headBlock);
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


    /// an opposite method -- offsets are transformed into pos-and-sizes.
    void offsetsToPosAndSize( const std::vector< offset_type >& offsets,
                              size_t dataSize,
                              std::vector< offset_type >& posAndSize )
    {
        const size_t usedBlocks = countBlocks(dataSize);
        assert( offsets.size() > 0 && offsets.size() >= usedBlocks );
        // one index + nav per block + extra index if there is free blocks
        posAndSize.resize( ( offsets.size() +
                             (usedBlocks == 0 ? 0 : 1) +    // if has used blocks
                             ((offsets.size() == usedBlocks) ? 0 : 1) // if has free blocks
                             ) * 2 );
        std::vector< offset_type >::iterator iter = posAndSize.begin();
        std::vector< offset_type >::const_iterator ofit = offsets.begin();
        if ( usedBlocks > 0 ) {
            *iter++ = notUsed(); // skip index
            *iter++ = 0;
            for ( ; dataSize > 0; ) {
                *iter++ = *ofit++;
                const size_t sz = std::min(dataSize,blockSize());
                *iter++ = sz;
                dataSize -= sz;
            }
        }
        if ( ofit != offsets.end() ) {
            // free blocks
            *iter++ = notUsed(); // skip index
            *iter++ = 0;
            for ( ; ofit != offsets.end(); ++ofit ) {
                *iter++ = *ofit;
                *iter++ = navSize();
            }
        }
        assert( iter == posAndSize.end() );
    }


    /// method prepares headers for used chain of length dataSize.
    void makeHeaders( buffer_type&              headers,
                      std::vector<offset_type>& offsets,
                      size_t                    dataSize )
    {
        // offsets contains affected block indices
        const size_t needBlocks = countBlocks(dataSize);
        if ( needBlocks > offsets.size() ) {
            throw smsc::util::Exception("too few offsets in makeHeaders");
        }

        headers.clear();
        headers.reserve(needBlocks*navSize()+idxSize());

        io::Serializer ser(headers);
        std::vector< offset_type >::const_iterator iter = offsets.begin();
        const offset_type firstBlock = *iter++;
        ser << firstBlock;
        BlockNavigation bn;
        bn.setDataSize( dataSize, packingType_ );
        for ( size_t i = 0; i < needBlocks; ++i ) {

            if ( dataSize == 0 ) {
                throw smsc::util::Exception("too many blocks in makeHeaders, first=%llx", firstBlock);
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
            throw smsc::util::Exception("too few blocks in makeHeaders, first=%llx", firstBlock );
        }

        if (log_ && log_->isDebugEnabled()) {
            HexDump hd;
            HexDump::string_type hex;
            if (headers.size()>0) { hd.hexdump(hex,&headers[0],headers.size()); }
            smsc_log_debug( log_,"headers made: %s", hd.c_str(hex));
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
        io::Serializer ser(profile);
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
            HexDump::string_type hex;
            hd.hexdump(hex,&profile[oldSize],profile.size()-oldSize);
            smsc_log_debug( log_,"free chain made: %s", hd.c_str(hex));
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
    inline size_t countBlocks( size_t dataSize ) const {
        if ( dataSize == 0 ) return 0;
        return (dataSize - 1) / blockSize() + 1;
    }

    /// block index to offset
    inline offset_type idx2pos( index_type idx ) const {
        if ( idx == invalidIndex() ) return notUsed_;
        return offset_type(idx*blockSize_);
    }

    inline index_type pos2idx( offset_type pos ) const {
        if ( (pos & BlockNavigation::badBit()) ) return invalidIndex();
        if ( pos % blockSize_ != 0 ) {
            throw smsc::util::Exception("offset %llx is not divisable by blockSize=%x",
                                        pos, unsigned(blockSize_) );
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
