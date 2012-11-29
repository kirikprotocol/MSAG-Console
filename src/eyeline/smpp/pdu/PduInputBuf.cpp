#include <assert.h>
#include <memory>
#include "PduInputBuf.h"
#include "BindPdu.h"
#include "GenericNackPdu.h"
#include "CancelSmPdu.h"
#include "QuerySmPdu.h"
#include "SubmitSmPdu.h"
#include "DeliverSmPdu.h"
#include "ReplaceSmPdu.h"
#include "UnbindPdu.h"
#include "OutbindPdu.h"
#include "EnquireLinkPdu.h"
#include "SubmitMultiPdu.h"
#include "AlertNotificationPdu.h"
#include "DataSmPdu.h"
#include "SmResponsePdu.h"


namespace eyeline {
namespace smpp {

PduInputBuf::PduInputBuf( size_t streamlen, const char* stream ) :
size_(streamlen), buf_(stream)
{
    if ( size_ < sizeof(uint32_t)*4 ) {
        throw SmppPduBadLenException("too small pdu input buffer len=%u",
                                     unsigned(size_));
    }
    eyeline::informer::FromBuf fb(buf_,size_);
    const uint32_t sz = fb.ncget32();
    if ( size_t(sz) != streamlen ) {
        throw SmppPduBadLenException("input buf len mismatch len=%u sz=%u",
                                     unsigned(streamlen), unsigned(sz));
    }
    pduTypeId_ = fb.ncget32();
    status_ = fb.ncget32();
    seqNum_ = fb.ncget32();
}


Pdu* PduInputBuf::decode() const
{
    // FIXME
    std::auto_ptr<Pdu> pdu(createPdu());
    if (pdu.get()) {
        
        const size_t sz = size_ - (sizeof(uint32_t)*4);
        eyeline::informer::FromBuf fb(buf_ + (sizeof(uint32_t)*4),sz);

        // mandatory size of empty pdu is just minimal pdu size
        if ( sz < pdu->mandatorySize() ) {
            throw SmppPduBadLenException("too small pdu(%s,seq=%u,len=%u) at decode mandatory",
                                         pduTypeToString(PduType(pduTypeId_)),
                                         unsigned(seqNum_),
                                         unsigned(size_));
        }
            
        try {
            pdu->decodeMandatoryFields(fb);
        } catch ( eyeline::informer::InfosmeException& e ) {
            throw SmppPduBadLenException("pdu(%s,seq=%u,len=%u) failed to decode mandatory",
                                         pduTypeToString(PduType(pduTypeId_)),
                                         unsigned(seqNum_),
                                         unsigned(size_));
        }
        pdu->optionals.decode(fb);
    }
    return pdu.release();
}


Pdu* PduInputBuf::createPdu() const
{
    Pdu* pdu = 0;

    switch ( pduTypeId_ ) {
    case GENERIC_NACK            : pdu = new GenericNackPdu(status_, seqNum_); break;

    case BIND_RECEIVER           : pdu = new BindPdu(BINDMODE_RECEIVER); break;
    case BIND_RECEIVER_RESP      : pdu = new BindRespPdu(BIND_RECEIVER_RESP, status_, seqNum_); break;
    case BIND_TRANSMITTER        : pdu = new BindPdu(BINDMODE_TRANSMITTER); break;
    case BIND_TRANSMITTER_RESP   : pdu = new BindRespPdu(BIND_TRANSMITTER_RESP, status_, seqNum_); break;
    case BIND_TRANSCEIVER        : pdu = new BindPdu(BINDMODE_TRANSCEIVER); break;
    case BIND_TRANSCEIVER_RESP   : pdu = new BindRespPdu(BIND_TRANSCEIVER_RESP, status_, seqNum_); break;

    case QUERY_SM                : pdu = new QuerySmPdu(); break;
    case QUERY_SM_RESP           : pdu = new QuerySmRespPdu(status_, seqNum_); break;
    case SUBMIT_SM               : pdu = new SubmitSmPdu(); break;
    case SUBMIT_SM_RESP          : pdu = new SmRespPdu(SUBMIT_SM_RESP, status_, seqNum_); break;
    case DELIVER_SM              : pdu = new DeliverSmPdu(); break;
    case DELIVER_SM_RESP         : pdu = new SmRespPdu(DELIVER_SM_RESP, status_, seqNum_); break;
    case UNBIND                  : pdu = new UnbindPdu(); break;
    case UNBIND_RESP             : pdu = new UnbindRespPdu(status_, seqNum_); break;
    case REPLACE_SM              : pdu = new ReplaceSmPdu(); break;
    case REPLACE_SM_RESP         : pdu = new ReplaceSmRespPdu(status_, seqNum_); break;
    case CANCEL_SM               : pdu = new CancelSmPdu(); break;
    case CANCEL_SM_RESP          : pdu = new CancelSmRespPdu(status_, seqNum_); break;
    case OUTBIND                 : pdu = new OutbindPdu(); break;
    case ENQUIRE_LINK            : pdu = new EnquireLinkPdu(); break;
    case ENQUIRE_LINK_RESP       : pdu = new EnquireLinkRespPdu(status_, seqNum_); break;
    case SUBMIT_MULTI            : pdu = new SubmitMultiPdu(); break;
    case SUBMIT_MULTI_RESP       : pdu = new SubmitMultiRespPdu(status_, seqNum_); break;
    case ALERT_NOTIFICATION      : pdu = new AlertNotificationPdu(); break;
    case DATA_SM                 : pdu = new DataSmPdu(); break;
    case DATA_SM_RESP            : pdu = new SmRespPdu(DATA_SM_RESP, status_, seqNum_); break;
    };
    if (pdu) {
        pdu->setStatus(status_);
        pdu->setSeqNum(seqNum_);
    }

    return pdu;
}


ResponsePdu* PduInputBuf::createResponse( PduType reqType,
                                          uint32_t seqNum,
                                          uint32_t status )
{
    PduType respType = pduTypeToResp(reqType);
    if (respType == PduType(0)) return 0;
    switch (respType) {
    case GENERIC_NACK            : return new GenericNackPdu(status, seqNum);
    case BIND_RECEIVER_RESP      : return new BindRespPdu(BIND_RECEIVER_RESP, status, seqNum);
    case BIND_TRANSMITTER_RESP   : return new BindRespPdu(BIND_TRANSMITTER_RESP, status, seqNum);
    case BIND_TRANSCEIVER_RESP   : return new BindRespPdu(BIND_TRANSCEIVER_RESP, status, seqNum);
    case QUERY_SM_RESP           : return new QuerySmRespPdu(status, seqNum);
    case SUBMIT_SM_RESP          : return new SmRespPdu(SUBMIT_SM_RESP, status, seqNum);
    case DELIVER_SM_RESP         : return new SmRespPdu(DELIVER_SM_RESP, status, seqNum);
    case UNBIND_RESP             : return new UnbindRespPdu(status, seqNum);
    case REPLACE_SM_RESP         : return new ReplaceSmRespPdu(status, seqNum);
    case CANCEL_SM_RESP          : return new CancelSmRespPdu(status, seqNum);
    case ENQUIRE_LINK_RESP       : return new EnquireLinkRespPdu(status, seqNum);
    case SUBMIT_MULTI_RESP       : return new SubmitMultiRespPdu(status, seqNum);
    case DATA_SM_RESP            : return new SmRespPdu(DATA_SM_RESP, status, seqNum);
    default:
        return 0;
    }
}

}
}
