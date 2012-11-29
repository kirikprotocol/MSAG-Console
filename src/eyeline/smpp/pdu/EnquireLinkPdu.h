#ifndef _EYELINE_SMPP_ENQUIRELINKPDU_H
#define _EYELINE_SMPP_ENQUIRELINKPDU_H

#include "RequestPdu.h"
#include "ResponsePdu.h"
#include "informer/io/IOConverter.h"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace smpp {


class EnquireLinkRespPdu : public ResponsePdu
{
public:
    EnquireLinkRespPdu( uint32_t status, uint32_t seqNum ) : ResponsePdu(status,seqNum) {
        pduTypeId_ = ENQUIRE_LINK_RESP;
    }
};


class EnquireLinkPdu : public RequestPdu
{
public:
    EnquireLinkPdu() : RequestPdu(ENQUIRE_LINK) {}

protected:
    size_t mandatorySize() const { return 0; }
};

}
}

#endif
