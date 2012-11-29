#ifndef _EYELINE_SMPP_REQUESTPDU_H
#define _EYELINE_SMPP_REQUESTPDU_H

#include "Pdu.h"

namespace eyeline {
namespace smpp {

class RequestPdu : public Pdu
{
protected:
    RequestPdu( uint32_t thetype ) {
        pduTypeId_ = thetype;
        status_ = 0;
    }
};

}
}

#endif
