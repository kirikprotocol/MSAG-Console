#ifndef _EYELINE_SMPP_GENERICNACKPDU_H
#define _EYELINE_SMPP_GENERICNACKPDU_H

#include "ResponsePdu.h"

namespace eyeline {
namespace smpp {


class GenericNackPdu : public ResponsePdu
{
public:
    GenericNackPdu(uint32_t status, uint32_t seqNum) : ResponsePdu(status, seqNum)
    {
        pduTypeId_ = GENERIC_NACK;
    }
};

}
}

#endif
