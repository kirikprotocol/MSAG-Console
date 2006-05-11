#ident "$Id$"
#ifndef _SMSC_INMAN_ERROR_CODES_HPP
#define _SMSC_INMAN_ERROR_CODES_HPP

#include <inttypes.h>

namespace smsc  {
namespace inman {

extern const char * _InmanErrorSource[];

typedef enum { errOk = 0, 
    errRPCause = 1,     // RP cause MO SM transfer
    errProtocol = 2,    // INMan <-> SMSC protocol
    errTCAP = 3,        // TCAP layer errors
    errTCuser = 4,      // TC user errors
    errCAP3 = 5,        // CAP3 services errors
    errCAPuser = 6,     // CAP related user errors
    errMAP = 7,         // MAP services errors
    errMAPuser = 8,     // MAP related user errors
    //...
    errUndefined        // reserved/undefined
} InmanErrorType;

typedef enum {
    InProtocol_GeneralError = 1,
    InProtocol_ResourceLimitation = 2,
    InProtocol_InvalidData = 3
} InmanProtocolErrors;

typedef enum {
  tcapUnrecognizedMessageType = 0, tcapUnrecognizedTransactionID = 1,
  tcapBadlyFormattedTransactionPortion = 2, tcapIncorrectTransactionPortion = 3,
  tcapResourceLimitation = 4
} InTCAPErrors;

class InmanErrorCode {
public:
    InmanErrorCode(uint32_t ercode);
    InmanErrorCode(InmanErrorType range, uint16_t ercode);

    //constructs InMan combined error code, returns zero on out-of-range args
    static uint32_t combineError(InmanErrorType range, uint16_t ercode);
    uint32_t        getCombinedError(void) const;
    //splits combined error to class and code
    void            splitError(InmanErrorType & errType, uint16_t & errCode);
    //returns class of error
    InmanErrorType  getErrorType(void) const;
    //returns original error code if error belongs to given InmanErrorType, otherwise returns zero
    uint16_t        getErrorCode(InmanErrorType range) const;

protected:
    uint32_t   _errcode; //combined error code
};

} //inman
} //smsc
#endif /* _SMSC_INMAN_ERROR_CODES_HPP */

