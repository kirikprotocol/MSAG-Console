#include <assert.h>
#include "Pdu.h"
#include "PduInputBuf.h"
#include "PduOptional.h"

namespace eyeline {
namespace smpp {

smsc::logger::Logger* Pdu::log_ = 0;
// uint32_t Pdu::globalSeqNum_ = 0;

namespace {
smsc::core::synchronization::Mutex logMutex;
smsc::core::synchronization::Mutex seqMutex;
}

void Pdu::initLog() 
{
    smsc::core::synchronization::MutexGuard mg(logMutex);
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("smpp.pdu");
    }
}


ResponsePdu* Pdu::createResponse( uint32_t status ) const
{
    return PduInputBuf::createResponse( PduType(pduTypeId_),
                                        seqNum_,
                                        status );
}


void Pdu::assignSeqNum() const
{
    if (seqNum_==uint32_t(-1)) {
        seqNum_ = PduBuffer::getNextSeqNum();
    }
}


PduBuffer Pdu::encode() const
{
    const size_t hintSize = evaluateSize();
    /*
     if ( buf.getSize() - buf.getPos() < hintSize ) {
     buf.reserve(buf.getPos()+hintSize);
     }
     */
    assignSeqNum();
    char dbuf[200];
    smsc_log_debug(log_,"encode %s bsz=%u",
                   toString(dbuf,sizeof(dbuf)),
                   unsigned(hintSize));
    PduBuffer buf(hintSize,seqNum_,isRequest());
    eyeline::informer::ToBuf tb(buf.getBuf(),buf.getSize());
    // header
    tb.skip(4); // skip the len
    tb.set32(pduTypeId_);
    tb.set32(status_);
    tb.set32(seqNum_);
    // mandatory fields
    encodeMandatoryFields(tb);
    // optional fields
    optionals.encode(tb);
    // set length
    const uint32_t sz = uint32_t(tb.getPos());
    assert(sz == hintSize);
    tb.setPos(0);
    tb.set32(sz);
    // buf.setPos(buf.getPos()+sz);
    return buf;
}


const char* Pdu::toString( char* buf, size_t bsz, size_t* written ) const
{
    // const size_t bsz = buf.getSize() - buf.getPos();
    size_t wr = 0;
    if ( bsz > 0 ) {
        int res = snprintf(buf,bsz,"pdu=%s,st=%x,seq=%x",
                           pduTypeToString(PduType(pduTypeId_)),
                           unsigned(status_),
                           unsigned(seqNum_));
        if ( res <= 0 ) {
            *buf = '\0';
            res = 0;
        } else if ( size_t(res) >= bsz ) {
            res = int(bsz-1);
        }
        wr += res;
    }
    if (written) *written = wr;
    return buf;
}


}
}
