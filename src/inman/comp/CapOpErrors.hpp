/* ************************************************************************* *
 * CAP operations error codes according to inman/codec_inc/cap_err_codes.asn
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_CAP_OPS_ERRORS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_CAP_OPS_ERRORS__

#include "util/URCdb/URCRegistry.hpp"

namespace smsc {
namespace inman {
namespace comp {

using smsc::util::URCSpacePTR;

/* ------------------------------------------------------------------------- *
 * CAP operations error codes.
 * ------------------------------------------------------------------------- */
struct CAPOpErrorId {
    enum Codes {
        canceled = 0,
        cancelFailed = 1,
        eTCFailed = 3,
        improperCallerResponse = 4,
        missingCustomerRecord = 6,
        missingParameter = 7,
        parameterOutOfRange = 8,
        requestedInfoError = 10,
        systemFailure = 11,
        taskRefused = 12,
        unavailableResource = 13,
        unexpectedComponentSequence = 14,
        unexpectedDataValue = 15,
        unexpectedParameter = 16,
        unknownLegID = 17,
        unknownPDPID = 50,
        unknownCSID = 51
    };
};
extern const char * rc2Txt_CapOpErrors(uint32_t err_code);
extern URCSpacePTR  _RCS_CAPOpErrors;

#define FDECL_rc2Txt_CapOpErrors()    \
const char * rc2Txt_CapOpErrors(uint32_t err_code) { \
    switch (err_code) { \
    case CAPOpErrorId::canceled: return "canceled"; \
    case CAPOpErrorId::cancelFailed: return "cancelFailed"; \
    case CAPOpErrorId::eTCFailed: return "eTCFailed"; \
    case CAPOpErrorId::improperCallerResponse: return "improperCallerResponse"; \
    case CAPOpErrorId::missingCustomerRecord: return "missingCustomerRecord"; \
    case CAPOpErrorId::missingParameter: return "missingParameter"; \
    case CAPOpErrorId::parameterOutOfRange: return "parameterOutOfRange"; \
    case CAPOpErrorId::requestedInfoError: return "requestedInfoError"; \
    case CAPOpErrorId::systemFailure: return "systemFailure"; \
    case CAPOpErrorId::taskRefused: return "taskRefused"; \
    case CAPOpErrorId::unavailableResource: return "unavailableResource"; \
    case CAPOpErrorId::unexpectedComponentSequence: return "unexpectedComponentSequence"; \
    case CAPOpErrorId::unexpectedDataValue: return "unexpectedDataValue"; \
    case CAPOpErrorId::unexpectedParameter: return "unexpectedParameter"; \
    case CAPOpErrorId::unknownLegID: return "unknownLegID"; \
    case CAPOpErrorId::unknownPDPID: return "unknownPDPID"; \
    case CAPOpErrorId::unknownCSID: return "unknownCSID"; \
    default:; } \
    return "unknown CAP Operation error code"; \
}

#define ODECL_RCS_CAPOpErrors() FDECL_rc2Txt_CapOpErrors() \
URCSpacePTR  _RCS_CAPOpErrors("errCAPOp", CAPOpErrorId::canceled, \
                                CAPOpErrorId::unknownCSID, rc2Txt_CapOpErrors)

/* ------------------------------------------------------------------------- *
 * General CAP service error codes.
 * ------------------------------------------------------------------------- */
struct CAPServiceRC {
    enum Error {
        //-- generic errors
        noServiceResponse = 1,  // CAP service doesn't respond
        contractViolation = 2,  // operation interchange violates CAP contract
        monitoringTimeOut = 3   // FSM monitoring state lasts too long
    };
};
extern const char * rc2Txt_CapService(uint32_t ret_code);
extern URCSpacePTR  _RCS_CAPService;

#define FDECL_rc2Txt_CapService()    \
const char * rc2Txt_CapService(uint32_t ret_code) { \
    switch (ret_code) { \
    case CAPServiceRC::noServiceResponse: return "CAP service not responding"; \
    case CAPServiceRC::contractViolation: return "CAP CONTRACT violation"; \
    case CAPServiceRC::monitoringTimeOut: return "FSM monitoring state lasts too long"; \
    default:; } \
    return "unknown CAP service error"; \
}

#define ODECL_RCS_CAPService() FDECL_rc2Txt_CapService() \
URCSpacePTR  _RCS_CAPService("errCAPSrv", CAPServiceRC::noServiceResponse, \
                    CAPServiceRC::contractViolation, rc2Txt_CapService)


/* ------------------------------------------------------------------------- *
 * Macro for all return code spaces global initialization
 * ------------------------------------------------------------------------- */
#define _RCS_CapOpErrorsINIT()  ODECL_RCS_CAPOpErrors(); ODECL_RCS_CAPService()

} //comp
} //inman
} //smsc

#define _RCS_CapOpErrorsGET() \
smsc::inman::comp::_RCS_CAPOpErrors.get(); \
smsc::inman::comp::_RCS_CAPService.get()

#endif /* __SMSC_INMAN_INAP_CAP_OPS_ERRORS__ */

