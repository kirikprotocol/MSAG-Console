#ident "$Id$"
#ifndef _SMSC_INMAN_ERROR_CODES_HPP
#define _SMSC_INMAN_ERROR_CODES_HPP

#include <inttypes.h>

namespace smsc  {
namespace inman {

typedef enum {
    InErrOk = 0, InErrRPCause = 1,
    InErrINprotocol = 2, InErrTCAP = 3, InErrCAP3 = 4
} InmanErrorType;

typedef enum {
    InProtocol_GeneralError = 1
} InmanProtocolErrors;

class InmanErrorCode {
public:
    InmanErrorCode(uint32_t ercode);
    InmanErrorCode(InmanErrorType range, uint16_t ercode);

    //constructs InMan combined error code, returns zero on out-of-range args
    static uint32_t  GetCombinedError(InmanErrorType range, uint16_t ercode);

    //returns class of error
    InmanErrorType  GetErrorType(void) const;
    //returns combined nonzero code holding RP cause or CAP3/TCAP error, or IN protocol error
    uint32_t        GetCombinedError(void) const;
    //returns original error code if error belongs to given InmanErrorClass, otherwise returns zero
    uint16_t        GetOrigError(InmanErrorType range) const;

    //returns nonzero RP cause MO SM transfer
    uint8_t         GetRPCause(void) const;
    //returns nonzero CAP3 error code
    uint16_t        GetCAP3Error(void) const;
    //returns nonzero TCAP error code
    uint8_t         GetTCAPError(void) const;
    //returns nonzero TCAP error code
    uint16_t        GetINprotocolError(void) const;

protected:
    uint32_t   _errcode; //combined error code
};

} //inman
} //smsc
#endif /* _SMSC_INMAN_ERROR_CODES_HPP */


