/* ************************************************************************* *
 * INMan error codes.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_ERRORS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_ERRORS__

#include "util/URCdb/URCRegistry.hpp"

namespace smsc {
namespace inman {

using smsc::util::URCSpacePTR;

struct INManErrorId {
    enum Code_e {
        noErr = 0,
        protocolGeneralError = 1,
        protocolInvalidData,
        cfgSpecific,
        cfgMismatch,
        cfgLimitation,
        cfgInconsistency,
        logicTimedOut,
        brokenPipe,
        srvInoperative,
        /* */
        internalError = 20 //this a max cap
    };
};
extern const char * rc2Txt_INManErrors(uint32_t err_code);
extern URCSpacePTR  _RCS_INManErrors;

#define FDECL_rc2Txt_INManErrors()    \
const char * rc2Txt_INManErrors(uint32_t err_code) { \
    switch (err_code) { \
    case INManErrorId::noErr:           return "Ok"; \
    case INManErrorId::protocolGeneralError: return "illegal/invalid PDU"; \
    case INManErrorId::protocolInvalidData: return "invalid or insufficient PDU data"; \
    case INManErrorId::cfgSpecific:         return "specific config settings"; \
    case INManErrorId::cfgMismatch:     return "config settings mismatch"; \
    case INManErrorId::cfgLimitation:   return "limitation according to config"; \
    case INManErrorId::cfgInconsistency:   return "config settings inconsistent or insufficient"; \
    case INManErrorId::logicTimedOut:   return "external operation is timed out"; \
    case INManErrorId::brokenPipe:      return "broken connection"; \
    case INManErrorId::srvInoperative:  return "requested service is inoperative"; \
    case INManErrorId::internalError:   return "internal error"; \
    default:; } \
    return "unknown INMan error code"; \
}

#define ODECL_RCS_INManErrors() FDECL_rc2Txt_INManErrors() \
URCSpacePTR  _RCS_INManErrors("errINMan", INManErrorId::noErr, \
                                INManErrorId::internalError, rc2Txt_INManErrors)

/* ------------------------------------------------------------------------- *
 * Macro for all return code spaces global initialization
 * ------------------------------------------------------------------------- */
#define _RCS_INManErrorsINIT()  ODECL_RCS_INManErrors()

} //inman
} //smsc

#define _RCS_INManErrorsGET() smsc::inman::_RCS_INManErrors.get()

#endif /* __SMSC_INMAN_ERRORS__ */

