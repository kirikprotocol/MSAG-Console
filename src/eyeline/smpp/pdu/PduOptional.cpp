#include <algorithm>
#include <netinet/in.h> // for ntohl
#include "PduOptional.h"
#include "eyeline/smpp/Utility.h"

namespace eyeline {
namespace smpp {

const uint32_t PduOptionals::mask[5] = {0, 0xff, 0xffff, 0xffffff, 0xffffffff};

PduOptionals::~PduOptionals()
{
    for ( TLVList::iterator i = tlvs_.begin(), ie = tlvs_.end();
          i != ie; ++i ) {
        if ( i->maxlen > sizeof(Value) ) {
            deallocate(*i);
        }
    }
}


bool PduOptionals::hasTag( uint16_t tag ) const 
{
    TLVList::const_iterator it =
        std::lower_bound(tlvs_.begin(), tlvs_.end(), tag, TLV::CompareTag());
    return it != tlvs_.end() && it->tag == tag;
}


void PduOptionals::remove( uint16_t tag )
{
    TLVList::iterator it =
        std::lower_bound(tlvs_.begin(), tlvs_.end(), tag, TLV::CompareTag());
    if ( (it != tlvs_.end()) && (it->tag == tag) ) {
        TLV& tmp = *it;
        if (tmp.len > sizeof(Value)) {
            deallocate(tmp);
        }
        tlvs_.erase(it);
    }
}

uint16_t PduOptionals::getLen( uint16_t tag ) const
{
    TLVList::const_iterator it =
        std::lower_bound(tlvs_.begin(), tlvs_.end(), tag, TLV::CompareTag());
    if ( (it == tlvs_.end()) || (it->tag != tag) )
        throw SmppTLVNotFoundException(tag,"TLV not found, tag=%x",tag);
    return it->len;
}

// may throw  TLVNotFoundException
uint32_t PduOptionals::getInt( uint16_t tag ) const 
{
    TLVList::const_iterator it = 
        std::lower_bound(tlvs_.begin(), tlvs_.end(), tag, TLV::CompareTag());
    if ( (it == tlvs_.end()) || (it->tag != tag) )
        throw SmppTLVNotFoundException(tag,"TLV not found, tag=%x",tag);
    uint32_t tmp = ntohl(it->value.intvalue);
    tmp >>= 8*(4-it->len);
    tmp &= mask[it->len];
    return tmp;
}

const char* PduOptionals::getString( uint16_t tag ) const 
{
    TLVList::const_iterator it = 
        std::lower_bound(tlvs_.begin(), tlvs_.end(), tag, TLV::CompareTag());
    if ( (it == tlvs_.end()) || (it->tag != tag) )
        throw SmppTLVNotFoundException(tag,"TLV not found, tag=%x",tag);
    return reinterpret_cast<const char*>(it->getPtr());
}


const void* PduOptionals::getBinary( uint16_t tag, size_t& len ) const 
{
    TLVList::const_iterator it =
        std::lower_bound(tlvs_.begin(), tlvs_.end(), tag, TLV::CompareTag());
    if ( (it == tlvs_.end()) || (it->tag != tag) )
        throw SmppTLVNotFoundException(tag,"TLV not found, tag=%x",tag);
    len = it->len;
    return reinterpret_cast<const void*>(it->getPtr());
}


void PduOptionals::setInt( uint16_t tag, uint16_t len, uint32_t value )
{
    TLVList::iterator it =
        std::lower_bound(tlvs_.begin(), tlvs_.end(), tag, TLV::CompareTag());
    if ( (it == tlvs_.end()) || (it->tag != tag) ) {
        it = tlvs_.insert(it, TLV(tag));
        it->maxlen = len;
        it->len = len;
    }
    uint32_t tmp = htonl(value & mask[len]);
    tmp >>= 8*(4-len);
    it->value.intvalue = tmp;
}

void PduOptionals::setString( uint16_t tag, const char* value ) 
{
    const size_t tmp_len = strlen(value) + 1;
    if (tmp_len > 0xffff) {
        throw SmppTLVBadLenException("tag=%x(%s), too big strlen=%llu",
                                     tag, TlvUtil::tlvName(tag),
                                     ulonglong(tmp_len));
    }
    this->setBinary(tag, uint16_t(tmp_len), reinterpret_cast<const void*>(value));
}

void PduOptionals::setBinary( uint16_t tag, uint16_t len, const void* value )
{
    if (len > TlvUtil::tlvMaxLength(tag) ) {
        throw SmppTLVBadLenException("wrong len on set TLV tag=%x(%s) len=%u maxlen=%u",
                                     tag, TlvUtil::tlvName(tag), len, TlvUtil::tlvMaxLength(tag));
    }
    TLVList::iterator it = 
        std::lower_bound(tlvs_.begin(), tlvs_.end(), tag, TLV::CompareTag());
    if ( (it == tlvs_.end()) || (it->tag != tag) ) {
        it = tlvs_.insert(it, TLV(tag));
        it->len = len;
    }
    TLV& tmp = *it;
    if (len > tmp.maxlen) {
        if (tmp.maxlen > sizeof(Value)) {
            deallocate(tmp);
        }
        allocate(len,tmp);
    }
    // tmp.len_ = len;
    memcpy(tmp.getPtr(), value, tmp.len);
}

void PduOptionals::encode( eyeline::informer::ToBuf& tb ) const 
{
    TLVList::const_iterator it = tlvs_.begin();
    for ( ; it != tlvs_.end(); ++it ) {
        tb.set16(it->tag);
        tb.set16(it->len);
        tb.copy(it->len, it->getPtr());
    }
}

// void PduOptionals::decode( const char* stream, size_t streamlen ) 
// {
//     eyeline::informer::FromBuf fb(stream, streamlen);
//     decode(fb);
// }

void PduOptionals::decode( eyeline::informer::FromBuf& fb ) 
{
    size_t streamlen = fb.getLen() - fb.getPos();
    size_t req_len = sizeof(uint16_t) + sizeof(uint16_t);
    while ( streamlen >= req_len ) {
        TLV tmp(fb.get16());
        TLVList::iterator it = std::lower_bound(tlvs_.begin(), tlvs_.end(), tmp);
        if ( it != tlvs_.end() && it->tag == tmp.tag ) {
            throw SmppException("decode optionals: duplicate tag=%u",tmp.tag);
        }
        tmp.len = fb.get16();
        if ( tmp.len ) {
            if ( streamlen < (req_len + tmp.len) ) {
                throw SmppTLVBadLenException("decode small buf for TLV tag=%x(%s) len=%u buflen=%u",
                                             tmp.tag, TlvUtil::tlvName(tmp.tag),
                                             tmp.len, unsigned(streamlen-req_len));
            }
            if ( tmp.len > TlvUtil::tlvMaxLength(tmp.tag) ) {
                throw SmppTLVBadLenException("decode wrong len TLV tag=%x(%s) len=%u maxlen=%u",
                                             tmp.tag, TlvUtil::tlvName(tmp.tag),
                                             tmp.len, TlvUtil::tlvMaxLength(tmp.tag) );
            }
            if ( (tmp.len > sizeof(Value)) ) {
                allocate(tmp.len, tmp);
            }
            memcpy(tmp.getPtr(), fb.skip(tmp.len), tmp.len);
        } else if ( tmp.len > TlvUtil::tlvMaxLength(tmp.tag) ) {
            throw SmppTLVBadLenException("decode wrong len TLV tag=%x(%s) len=%u maxlen=%u",
                                         tmp.tag, TlvUtil::tlvName(tmp.tag),
                                         tmp.len, TlvUtil::tlvMaxLength(tmp.tag) );
        }
        tlvs_.insert(it,tmp);
        streamlen -= req_len + tmp.len;
    }
}


size_t PduOptionals::evaluateSize() const
{
    size_t res = 0;
    for ( TLVList::const_iterator i = tlvs_.begin(), ie = tlvs_.end();
          i != ie; ++i ) {
        res += 4 + i->len;
    }
    return res;
}


void PduOptionals::allocate( uint16_t len, TLV& tlv ) 
{
    /*
     * NOTE: this check is not needed here (it's already done elsewhere)
    if (len > TlvUtil::tlvMaxLength(tlv.tag) ) {
        throw TLVBadDataException("TLV bad data for allocate: tag=%x, len=%d maxlen=%d %s",
                                  tlv.tag, len, TlvUtil::tlvMaxLength(tlv.tag), TlvUtil::tlvName(tlv.tag));
    }
     */
    typedef eyeline::informer::MemoryPool MemoryPool;
    tlv.len = len;
#ifdef SMPPPDUUSEMEMORYPOOL
    MemoryPool::SizeCache* c = PduBuffer::pool.sizeToCache(len);
    if (!c) {
        tlv.maxlen = len;
        tlv.ptr = reinterpret_cast<Value*>(PduBuffer::pool.allocate(tlv.len));
    } else {
        tlv.maxlen = c->maxsize > 0xffff ? uint16_t(0xffff) : uint16_t(c->maxsize);
        tlv.ptr = reinterpret_cast<Value*>(PduBuffer::pool.allocate(c,len));
    }
#else
    tlv.maxlen = len;
    tlv.ptr = reinterpret_cast<Value*>(::operator new(len));
#endif
}

void PduOptionals::deallocate( TLV& tlv ) 
{
    if ( tlv.maxlen <= sizeof(Value) )
        throw SmppTLVBadLenException("TLV dealloc logic error: tag=%x(%s), maxlen=%u",
                                     tlv.tag, TlvUtil::tlvName(tlv.tag), tlv.maxlen);
#ifdef SMPPPDUUSEMEMORYPOOL
    PduBuffer::pool.deallocate(tlv.ptr,tlv.maxlen);
#else
    ::operator delete(tlv.ptr);
#endif
    tlv.maxlen = uint16_t(sizeof(Value));
    tlv.ptr = 0;
}

}}  //namespace
