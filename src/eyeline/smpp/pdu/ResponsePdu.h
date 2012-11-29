#ifndef _EYELINE_SMPP_RESPONSEPDU_H
#define _EYELINE_SMPP_RESPONSEPDU_H

#include "Pdu.h"

namespace eyeline {
namespace smpp {

class ResponsePdu : public Pdu
{
public:
    ResponsePdu( uint32_t status, uint32_t seqNum ) : Pdu(seqNum)
    {
        status_ = status;
    }

protected:
    size_t mandatorySize() const { return 0; }
private:
    ResponsePdu& operator = ( const ResponsePdu& );
    ResponsePdu( const ResponsePdu& p );
};

}
}

#endif
