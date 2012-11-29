#ifndef _EYELINE_SMPP_PDUOPTIONAL_H
#define _EYELINE_SMPP_PDUOPTIONAL_H

#include <vector>
#include "TlvDefines.h"
#include "informer/io/IOConverter.h"
#include "informer/io/InfosmeException.h"
#include "eyeline/smpp/SmppException.h"
#include "PduBuffer.h"
#include "informer/io/StdAlloc.h"
#include "util/int.h"

namespace eyeline {
namespace smpp {

struct PduOptionals
{
#ifdef SMPPPDUUSEMEMORYPOOL
    static inline void* operator new ( size_t sz ) {
        return PduBuffer::pool.allocate(sz);
    }
    static inline void operator delete ( void* ptr, size_t sz ) {
        PduBuffer::pool.deallocate(ptr,sz);
    }
#endif

    ~PduOptionals();

    bool hasTag( uint16_t tag ) const;

    void remove( uint16_t tag );

    uint16_t getLen( uint16_t tag ) const;

    // may throw TLVNotFoundException
    uint32_t getInt( uint16_t tag ) const;
    const char* getString( uint16_t tag ) const;
    const void* getBinary( uint16_t tag, size_t& len ) const;

    void setInt( uint16_t tag, uint16_t len, uint32_t value );
    // may throw TLVNotFoundException, TLVBadDataException
    void setString( uint16_t tag, const char* value );
    void setBinary( uint16_t tag, uint16_t len, const void* value );

    void encode( eyeline::informer::ToBuf& tb ) const;
    void decode( eyeline::informer::FromBuf& fb );
    // void decode( const char* stream, size_t streamlen );
    size_t evaluateSize() const;
    const size_t count() const { return tlvs_.size(); }

protected:
    struct Value 
    {
        union {
            char     value[8];     // actually it may be bigger according to len
            uint32_t intvalue;
        };
    };

    struct TLV 
    {
        union {
            Value    value;
            Value*   ptr;
        };
        uint16_t maxlen;  // used at allocation only, do not change
        uint16_t tag;
        uint16_t len;
        uint8_t  type;

        TLV(uint16_t t) : maxlen(sizeof(Value)), tag(t) {
            type = TlvUtil::tlvDataType(tag);
        }

        inline const Value* getPtr() const {
            return ( maxlen <= sizeof(Value) ? &value : ptr );
        }
        inline Value* getPtr() {
            return ( maxlen <= sizeof(Value) ? &value : ptr );
        }
        inline bool operator < ( const TLV& o ) const { return tag < o.tag; }
        inline bool operator == ( const TLV& o ) const { return tag == o.tag; }

        struct CompareTag
        {
            bool operator () (const TLV& left, const uint16_t tag) {
                return left.tag < tag;
            }
        };
    };

    // may throw TLVBadDataException
    static void allocate( uint16_t len, TLV& tlv );
    static void deallocate( TLV& tlv );

#ifdef SMPPPDUUSEMEMORYPOOL
    typedef std::vector<TLV, eyeline::informer::StdAlloc<TLV,PduBuffer> > TLVList;
#else
    typedef std::vector<TLV> TLVList;
#endif
    TLVList tlvs_;
    static const uint32_t mask[5];
};


}
}

#endif
