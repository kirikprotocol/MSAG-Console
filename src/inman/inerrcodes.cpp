static char const ident[] = "$Id$";

#include "inman/inerrcodes.hpp"
#include <assert.h>

namespace smsc  {
namespace inman {

const char * _InmanErrorSource[] = {
    "Ok", "RPCause", "INManInternals",
    "TCAP", "TCuser", "CAP3", "CAPuser", "MAP", "MAPuser"
};
/* ************************************************************************** *
 * class InmanErrorCode implementation:
 * ************************************************************************** */
typedef struct {
    uint32_t  base;
    uint32_t  limit;
} InErrorRange;

static const InErrorRange    _ranges[] = { {0,0} //placeholder
    ,{0, 255}      //RP cause MO SM transfer
    ,{256, 279}    //Inman logic/protocol errors
    ,{280, 409}    //TCAP errors
    ,{410, 675}    //TCuser  errors
    ,{680, 935}    //CAP3 operations errors
    ,{940, 1195}   //CAP user errors
    ,{1200, 1455}  //MAP operations errors
    ,{1460, 1715}  //MAP user errors
    ,{1720, 0xFFFF} //reserved
};
#define MAX_RANGE (sizeof(_ranges)/sizeof(InErrorRange) - 1)

InmanErrorCode::InmanErrorCode(uint32_t ercode)
    : _errcode(ercode)
{
    assert(_errcode <= _ranges[MAX_RANGE].limit);
}
InmanErrorCode::InmanErrorCode(InmanErrorType range, uint16_t ercode)
{
    assert(range <= MAX_RANGE);
    _errcode = _ranges[range].base + ercode;
    assert(_errcode <= _ranges[range].limit);
}

//constructs InMan combined error code, returns zero on out-of-range args
uint32_t  InmanErrorCode::combineError(InmanErrorType range, uint16_t ercode)
{
    uint32_t errCode = _ranges[range].base + ercode;
    return (errCode > _ranges[range].limit) ? 0 : errCode;
}

InmanErrorType  InmanErrorCode::setError(uint32_t err_code)
{
    _errcode = err_code;
    uint16_t sErrc;
    return splitError(sErrc);
}

//splits combined error to class and code
InmanErrorType InmanErrorCode::splitError(uint16_t & errCode)
{
    for (int range = 0; range <= MAX_RANGE; range++) {
        if ((_errcode >= _ranges[range].base)
            && (_errcode <= _ranges[range].limit)) {
            errCode = _errcode - _ranges[range].base;
            return (InmanErrorType)range;
        }
    }
    //should never be reached
    errCode = 0;
    return errUndefined;
}

//returns class of error
InmanErrorType InmanErrorCode::getErrorType(void) const
{
    for (int range = 0; range <= MAX_RANGE; range++) {
        if ((_errcode >= _ranges[range].base)
            && (_errcode <= _ranges[range].limit))
            return (InmanErrorType)range;
    }
    return errOk; //should never be reached (checked in constructor)
}

//returns original error code if error belongs to given InmanErrorClass, otherwise returns zero
uint16_t InmanErrorCode::getErrorCode(InmanErrorType range) const
{
    if ((_errcode >= _ranges[range].base)
        && (_errcode <= _ranges[range].limit))
        return (uint16_t)(_errcode - _ranges[range].base);
    return 0;
}

} //inman
} //smsc

