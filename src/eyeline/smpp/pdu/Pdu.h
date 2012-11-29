#ifndef _EYELINE_SMPP_PDU_H
#define _EYELINE_SMPP_PDU_H

#include <vector>
#include "logger/Logger.h"
#include "informer/io/IOConverter.h"
#include "PduOptional.h"
#include "Types.h"

namespace eyeline {
namespace smpp {

class ResponsePdu;
class PduInputBuf;

/// this is a base class for SMPP pdu
/// (a stub to be filled)
class Pdu
{
    friend class PduInputBuf;
protected:
    static smsc::logger::Logger*         log_;

    void initLog();

public:
#ifdef SMPPPDUUSEMEMORYPOOL
    static inline void* operator new ( size_t sz ) {
        return PduBuffer::pool.allocate(sz);
    }
    static inline void operator delete ( void* ptr, size_t sz ) {
        PduBuffer::pool.deallocate(ptr,sz);
    }
#endif

    inline Pdu( uint32_t seqNum = uint32_t(-1) ) : seqNum_(seqNum) 
    {
        if (!log_) { initLog(); }
        // smsc_log_debug(log_,"ctor %p",this);
    }

    virtual ~Pdu() {
        // smsc_log_debug(log_,"dtor %p",this);
    }

    // virtual ResponsePdu* createResponse( uint32_t status = Status::OK ) const = 0;
    ResponsePdu* createResponse( uint32_t status = Status::OK ) const;

    inline uint32_t getSeqNum() const { return seqNum_; }
    inline uint32_t getPduTypeId() const { return pduTypeId_; }
    inline uint32_t getStatus() const { return status_; }

    inline void setSeqNum(const uint32_t value) { seqNum_ = value; }

    /// NOTE: this method is made constant because
    /// it only sets seqNum if it was not set before.
    void assignSeqNum() const;

    inline void setStatus(const uint32_t value) { status_ = value; }

    inline bool isRequest() const { return !(pduTypeId_ & 0x80000000); }

    PduBuffer encode() const;

    virtual const char* toString( char* buf, size_t bsz, size_t* written = 0 ) const; 

    size_t evaluateSize() const {
        return 4*sizeof(uint32_t) + mandatorySize() + optionals.evaluateSize();
    }

protected:
    virtual size_t mandatorySize() const = 0;
    virtual void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const {}
    virtual void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) {}

protected:
    uint32_t pduTypeId_; // actually it is not needed (but useful for debugging)
    uint32_t status_;
    mutable uint32_t seqNum_;    // allowed 0x1 .. 0x7fffffff (-1 if not set)

public:
    PduOptionals optionals;
    void removeTlv(uint16_t tag) { optionals.remove(tag); }
};

}
}

#endif
