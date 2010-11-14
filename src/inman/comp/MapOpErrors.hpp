/* ************************************************************************* *
 * MAP operations error codes according to inman/codec_inc/map_errors.asn
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_MAP_OPS_ERRORS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_MAP_OPS_ERRORS__

#include "util/URCdb/URCRegistry.hpp"

namespace smsc {
namespace inman {
namespace comp {

using smsc::util::URCSpacePTR;

/* ------------------------------------------------------------------------- *
 * MAP operations error codes.
 * ------------------------------------------------------------------------- */
struct MAPOpErrorId {
    enum Codes {
        //-- generic errors
        systemFailure = 34,
        dataMissing = 35,
        unexpectedDataValue = 36,
        facilityNotSupported = 21,
        //-- identification and numbering errors
        unknownSubscriber = 1,
        numberChanged = 44,
        //-- subscription errors
        bearerServiceNotProvisioned = 10,
        teleserviceNotProvisioned = 11,
        //-- call handling errors
        absentSubscriber = 27,
        busySubscriber = 45,
        noSubscriberReply = 46,
        callBarred = 13,
        forwardingViolation = 14,
        cug_Reject = 15,
        or_NotAllowed = 48,
        //-- any time information handling errors
        atsi_NotAllowed = 60,
        informationNotAvailable = 62,
        //-- supplementary service errors
        illegalSS_Operation = 16,
        ss_NotAvailable = 18,
        unknownAlphabet = 71
    };
};
extern const char * rc2Txt_MapOpErrors(uint32_t err_code);
extern URCSpacePTR  _RCS_MAPOpErrors;

#define FDECL_rc2Txt_MapOpErrors()    \
const char * rc2Txt_MapOpErrors(uint32_t err_code) { \
    switch (err_code) { \
    /* -- generic errors */ \
    case MAPOpErrorId::systemFailure: return "systemFailure"; \
    case MAPOpErrorId::dataMissing: return "dataMissing"; \
    case MAPOpErrorId::unexpectedDataValue: return "unexpectedDataValue"; \
    case MAPOpErrorId::facilityNotSupported: return "facilityNotSupported"; \
    /* -- identification and numbering errors */ \
    case MAPOpErrorId::unknownSubscriber: return "unknownSubscriber"; \
    case MAPOpErrorId::numberChanged: return "numberChanged"; \
    /* -- subscription errors */ \
    case MAPOpErrorId::bearerServiceNotProvisioned: return "bearerServiceNotProvisioned"; \
    case MAPOpErrorId::teleserviceNotProvisioned: return "teleserviceNotProvisioned"; \
    /* -- call handling errors */ \
    case MAPOpErrorId::absentSubscriber: return "absentSubscriber"; \
    case MAPOpErrorId::busySubscriber: return "busySubscriber"; \
    case MAPOpErrorId::noSubscriberReply: return "noSubscriberReply"; \
    case MAPOpErrorId::callBarred: return "callBarred"; \
    case MAPOpErrorId::forwardingViolation: return "forwardingViolation"; \
    case MAPOpErrorId::cug_Reject: return "cug-Reject"; \
    case MAPOpErrorId::or_NotAllowed: return "or-NotAllowed"; \
    /* -- any time information handling errors */ \
    case MAPOpErrorId::atsi_NotAllowed: return "atsi-NotAllowed"; \
    case MAPOpErrorId::informationNotAvailable: return "informationNotAvailable"; \
    /* -- supplementary service errors */ \
    case MAPOpErrorId::illegalSS_Operation: return "illegalSS-Operation"; \
    case MAPOpErrorId::ss_NotAvailable: return "ss-NotAvailable"; \
    case MAPOpErrorId::unknownAlphabet: return "unknownAlphabet"; \
    default:; } \
    return "unknown MAP Operation error code"; \
}

#define ODECL_RCS_MAPOpErrors() FDECL_rc2Txt_MapOpErrors() \
URCSpacePTR  _RCS_MAPOpErrors("errMAPOp", 1, 71, rc2Txt_MapOpErrors)

/* ------------------------------------------------------------------------- *
 * General MAP service error codes.
 * ------------------------------------------------------------------------- */
struct MAPServiceRC {
    enum Error { 
        //-- generic errors
        noServiceResponse = 1
    };
};
extern const char * rc2Txt_MapService(uint32_t ret_code);
extern URCSpacePTR  _RCS_MAPService;

#define FDECL_rc2Txt_MapService()    \
const char * rc2Txt_MapService(uint32_t ret_code) { \
    switch (ret_code) { \
    case MAPServiceRC::noServiceResponse: return "MAP service not responding"; \
    default:; } \
    return "unknown MAP service error"; \
}

#define ODECL_RCS_MAPService() FDECL_rc2Txt_MapService() \
URCSpacePTR  _RCS_MAPService("errMAPSrv", MAPServiceRC::noServiceResponse, \
                    MAPServiceRC::noServiceResponse, rc2Txt_MapService)


/* ------------------------------------------------------------------------- *
 * Macro for all return code spaces global initialization
 * ------------------------------------------------------------------------- */
#define _RCS_MapOpErrorsINIT()  ODECL_RCS_MAPOpErrors(); ODECL_RCS_MAPService()


} //comp
} //inman
} //smsc

#define _RCS_MapOpErrorsGET() \
smsc::inman::comp::_RCS_MAPOpErrors.get(); \
smsc::inman::comp::_RCS_MAPService.get()

#endif /* __SMSC_INMAN_INAP_MAP_OPS_ERRORS__ */

