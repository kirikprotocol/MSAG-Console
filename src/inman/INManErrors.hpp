#ident "$Id$"
/* ************************************************************************* *
 * INMan error codes.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_ERRORS__
#define __SMSC_INMAN_ERRORS__

#include "util/URCdb/URCRegistry.hpp"
using smsc::util::URCSpacePTR;

namespace smsc {
namespace inman {

struct INManErrorId {
    enum Codes {
        protocolGeneralError = 1,
        protocolInvalidData,
        cfgSpecific,
        cfgMismatch,
        cfgLimitation,
        cfgInconsistency,
        logicTimedOut,
        brokenPipe
    };
};
extern const char * rc2Txt_INManErrors(uint32_t err_code);
extern URCSpacePTR  _RCS_INManErrors;

#define FDECL_rc2Txt_INManErrors()    \
const char * rc2Txt_INManErrors(uint32_t err_code) { \
    switch (err_code) { \
    case INManErrorId::protocolGeneralError: return "illegal/invalid PDU"; \
    case INManErrorId::protocolInvalidData: return "invalid or insufficient PDU data"; \
    case INManErrorId::cfgSpecific:         return "specific config settings"; \
    case INManErrorId::cfgMismatch:     return "config settings mismatch"; \
    case INManErrorId::cfgLimitation:   return "limitation according to config"; \
    case INManErrorId::cfgInconsistency:   return "config settings inconsistent or insufficient"; \
    case INManErrorId::logicTimedOut:   return "external operation is timed out"; \
    case INManErrorId::brokenPipe:      return "broken connection"; \
    default:; } \
    return "unknown INMan error code"; \
}

#define ODECL_RCS_INManErrors() FDECL_rc2Txt_INManErrors() \
URCSpacePTR  _RCS_INManErrors("errINMan", INManErrorId::protocolGeneralError, \
                                INManErrorId::logicTimedOut, rc2Txt_INManErrors)

/* ------------------------------------------------------------------------- *
 * Macro for all return code spaces global initialization
 * ------------------------------------------------------------------------- */
#define _RCS_INManErrorsINIT()  ODECL_RCS_INManErrors()

} //inman
} //smsc
#endif /* __SMSC_INMAN_ERRORS__ */

