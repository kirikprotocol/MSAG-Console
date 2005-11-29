static char const ident[] = "$Id$";

#include "inman/interaction/inerrcodes.hpp"
#include <assert.h>

namespace smsc  {
namespace inman {
/* ************************************************************************** *
 * class InmanErrorCode implementation:
 * ************************************************************************** */
#define MAX_RANGE InErrCAP3
typedef struct {
    uint32_t  base;
    uint32_t  limit;
} InErrorRange;

static const InErrorRange    _ranges[] = { {0,0} //placeholder
    , {0, 255}      //RP cause MO SM transfer
    , {256, 279}    //Inman TCP protocol errors
    , {280, 409}    //TCAP errors
    , {410, 0xFFFF} //CAP3 operations errors
};

InmanErrorCode::InmanErrorCode(uint32_t ercode)
    : _errcode(ercode)
{
    assert(_errcode <= _ranges[MAX_RANGE].limit);
}
InmanErrorCode::InmanErrorCode(InmanErrorType range, uint16_t ercode)
{
    _errcode = _ranges[range].base + ercode;
    assert(_errcode <= _ranges[range].limit);
}

//constructs InMan combined error code, returns zero on out-of-range args
uint32_t  InmanErrorCode::GetCombinedError(InmanErrorType range, uint16_t ercode)
{
    uint32_t errCode = _ranges[range].base + ercode;
    if (errCode > _ranges[range].limit)
        return 0;
}

//returns class of error
InmanErrorType InmanErrorCode::GetErrorType(void) const
{
    for (int range = 0; range <= MAX_RANGE; range++) {
        if ((_errcode >= _ranges[range].base)
            && (_errcode <= _ranges[range].limit))
            return (InmanErrorType)range;
    }
}

//returns combined nonzero code holding RP cause or CAP3 error, or protocol error
uint32_t InmanErrorCode::GetCombinedError(void) const
{
    return _errcode;
}

//returns original error code if error belongs to given InmanErrorClass, otherwise returns zero
uint16_t InmanErrorCode::GetOrigError(InmanErrorType range) const
{
    if ((_errcode >= _ranges[range].base)
        && (_errcode <= _ranges[range].limit))
        return (uint16_t)(_errcode - _ranges[range].base);
    return 0;
}

//returns nonzero RP cause MO SM transfer
uint8_t  InmanErrorCode::GetRPCause(void) const
{
    return (uint8_t)GetOrigError(InErrRPCause);
}

//returns nonzero CAP3 error code
uint16_t InmanErrorCode::GetCAP3Error(void) const
{
    return (uint16_t)GetOrigError(InErrCAP3);
}
//returns nonzero TCAP error code
uint8_t  InmanErrorCode::GetTCAPError(void) const
{
    return (uint8_t)GetOrigError(InErrTCAP);
}

//returns nonzero TCAP error code
uint16_t InmanErrorCode::GetINprotocolError(void) const
{
    return (uint16_t)GetOrigError(InErrINprotocol);
}

} //inman
} //smsc

