#ifndef _EYELINE_SMPP_UNBINDPDU_H
#define _EYELINE_SMPP_UNBINDPDU_H

#include "RequestPdu.h"
#include "ResponsePdu.h"
#include "informer/io/IOConverter.h"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace smpp {

class UnbindRespPdu : public ResponsePdu
{
public:
    UnbindRespPdu( uint32_t status, uint32_t seqNum ) : ResponsePdu(status,seqNum) {
        pduTypeId_ = UNBIND_RESP;
    }
};


class UnbindPdu : public RequestPdu
{
public:
    UnbindPdu() : RequestPdu(UNBIND) {}

protected:
    size_t mandatorySize() const { return 0; }
};

}
}

#endif
