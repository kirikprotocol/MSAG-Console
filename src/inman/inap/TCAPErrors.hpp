/* ************************************************************************* *
 * Various TC provider return/error codes.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_TCAP_ERRORS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_TCAP_ERRORS__

#include "inman/inap/SS7MsgErrors.hpp"
#include "i97tcapapi.h"

namespace smsc {
namespace inman {
namespace inap {

/* ------------------------------------------------------------------------- *
 * TC Abort Cause: indicates the reason why transaction is aborted by the
 * TCAP provider (returned by T_PABORT_IND).
 * ------------------------------------------------------------------------- */
struct TC_PAbortCause {
    enum {
        //Q.773 P-AbortCause:
        unrecognizedMessageType = EINSS7_I97TCAP_ABORT_UNREC_MES,
        unrecognizedTransactionID = EINSS7_I97TCAP_ABORT_UNREC_ID,
        badFormatTransactionPortion = EINSS7_I97TCAP_ABORT_BAD_TP,
        incorrectTransactionPortion = EINSS7_I97TCAP_ABORT_INCORRECT_TP,
        resourceLimitation = EINSS7_I97TCAP_ABORT_RESOURCE_LIMIT,
        //EINSS7_I97TCAP additional codes
        abnormalDialogue = 0x05 /*EINSS7_I97TCAP_ABORT_ABNORMAL_DIALOGUE*/
    };
};
extern const char * rc2Txt_TC_PAbortCause(uint32_t rc_code);
extern URCSpacePTR  _RCS_TC_PAbort;

#define FDECL_rc2Txt_TC_PAbortCause()    \
const char * rc2Txt_TC_PAbortCause(uint32_t rc_code) { \
    switch (rc_code) { \
    case TC_PAbortCause::unrecognizedMessageType: return "Unrecognized message type"; \
    case TC_PAbortCause::unrecognizedTransactionID: return "Unrecognized Transaction ID"; \
    case TC_PAbortCause::badFormatTransactionPortion: return "Badly formatted Transaction Portion"; \
    case TC_PAbortCause::incorrectTransactionPortion: return "Incorrect Transaction Portion"; \
    case TC_PAbortCause::resourceLimitation: return "Resource Limitation"; \
    case TC_PAbortCause::abnormalDialogue: return "Abnormal Dialogue"; \
    default:; } \
    return "unknown TC PAbort cause"; \
}

#define ODECL_RCS_TC_PAbortCause() FDECL_rc2Txt_TC_PAbortCause() \
URCSpacePTR  _RCS_TC_PAbort("errTCPAbort", TC_PAbortCause::unrecognizedMessageType, \
                    TC_PAbortCause::abnormalDialogue, rc2Txt_TC_PAbortCause)

/* ------------------------------------------------------------------------- *
 * TC User Abort Cause: indicates the reason why transaction is aborted by
 * the TCAP user (returned by T_UABORT_IND).
 * ------------------------------------------------------------------------- */
struct TC_UAbortCause {
    enum {
        //Q.773 Associate-source-diagnostic : dialogue-service-user
        null = 0, no_reason_given = 1,
        application_context_not_supported = 2,
        //additional codes
        userDefinedAS
    };
};
extern const char * rc2Txt_TC_UAbortCause(uint32_t rc_code);
extern URCSpacePTR  _RCS_TC_UAbort;

#define FDECL_rc2Txt_TC_UAbortCause()    \
const char * rc2Txt_TC_UAbortCause(uint32_t rc_code) { \
    switch (rc_code) { \
    case TC_UAbortCause::null: return "0"; \
    case TC_UAbortCause::no_reason_given: return "no reason given"; \
    case TC_UAbortCause::application_context_not_supported: return "application context not supported"; \
    case TC_UAbortCause::userDefinedAS: return "User abort(ABRT APDU or user-defined AS)"; \
    default:; } \
    return "unknown TC UAbort cause"; \
}

#define ODECL_RCS_TC_UAbortCause() FDECL_rc2Txt_TC_UAbortCause() \
URCSpacePTR  _RCS_TC_UAbort("errTCUAbort", TC_UAbortCause::null, \
                    TC_UAbortCause::userDefinedAS, rc2Txt_TC_UAbortCause)

/* ------------------------------------------------------------------------- *
 * TC Bind Result: TC subsystem bind request error codes
 * ------------------------------------------------------------------------- */
struct TC_BindResult {
    enum {
        bindOk = EINSS7_I97TCAP_BIND_OK,
#ifndef EIN_HD
        ssnInUse = EINSS7_I97TCAP_BIND_SSN_IN_USE,
#elif EIN_HD >= 101
        ssnInUse = EINSS7_I97TCAP_BIND_VALUE_IN_USE,
#else
        ssnInUse = 1,
#endif /* EIN_HD >= 101 */
        protocolError = EINSS7_I97TCAP_BIND_PROTOCOL_ERR,
        noResources = EINSS7_I97TCAP_BIND_NO_RESOURCES,
        invalidSSN = EINSS7_I97TCAP_BIND_INVALID_SSN,
        sccpNotReady = EINSS7_I97TCAP_BIND_SCCP_NOT_READY,
#ifndef EIN_HD
        tcUserIdErr = EINSS7_I97TCAP_BIND_USER_ID_ERR,
        bindIfaceErr = 7,
#elif EIN_HD >= 101
        tcUserIdErr = EINSS7_I97TCAP_BIND_INVALID_USER_ID,
        bindIfaceErr = EINSS7_I97TCAP_BIND_INTERFACE_ERR,
#endif /* EIN_HD >= 101 */
        unknownErr
    };
};
extern const char * rc2Txt_TC_BindResult(uint32_t rc_code);
extern URCSpacePTR  _RCS_TC_Bind;

#define FDECL_rc2Txt_TC_BindResult()    \
const char * rc2Txt_TC_BindResult(uint32_t rc_code) { \
    switch (rc_code) { \
    case TC_BindResult::bindOk: return "Bind successful"; \
    case TC_BindResult::ssnInUse: return "Subsystem already in use"; \
    case TC_BindResult::protocolError: return "Protocol error"; \
    case TC_BindResult::noResources: return "Resources unavailable"; \
    case TC_BindResult::invalidSSN: return "Invalid subsystem number"; \
    case TC_BindResult::sccpNotReady: return "SCCP not ready"; \
    case TC_BindResult::tcUserIdErr: return "The userId is invalid or the TC-user is already bound"; \
    case TC_BindResult::bindIfaceErr: return "invalid TCBind() usage"; \
    default:; } \
    return "unknown TCBind() failure cause"; \
}

#define ODECL_RCS_TC_Bind() FDECL_rc2Txt_TC_BindResult() \
URCSpacePTR  _RCS_TC_Bind("errTCBind", TC_BindResult::bindOk, \
                    TC_BindResult::unknownErr, rc2Txt_TC_BindResult)

/* ------------------------------------------------------------------------- *
 * TC Report Cause: indicates the reason why a message was returned
 * (transaction cann't be completed by TC provider) (returned by T_NOTICE_IND)
 * ------------------------------------------------------------------------- */
struct TC_ReportCause {
    enum {
        noAdrNatureTranslation = EINSS7_I97TCAP_REP_NO_TR_ADR_NATURE,
        noAdrTranslation = EINSS7_I97TCAP_REP_NO_TR_ADR,
        ssnCongest = EINSS7_I97TCAP_REP_SUBSYST_CONGEST,
        ssnFailure = 3, //obsolete EINSS7_I97TCAP_REP_SUBSYST_FAILURE,
        unequipUser = EINSS7_I97TCAP_REP_UNEQUIP_USER,
        networkFailure = EINSS7_I97TCAP_REP_NETW_FAILURE,
        networkCongest = EINSS7_I97TCAP_REP_NETW_CONGEST,
        unqualified = EINSS7_I97TCAP_REP_UNQUALIFIED,
        hopCountViolation = EINSS7_I97TCAP_REP_HOPCOUNT_VIOL
    };
};
extern const char * rc2Txt_TC_ReportCause(uint32_t rc_code);
extern URCSpacePTR  _RCS_TC_Report;

#define FDECL_rc2Txt_TC_ReportCause()    \
const char * rc2Txt_TC_ReportCause(uint32_t rc_code) { \
    switch (rc_code) { \
    case TC_ReportCause::noAdrNatureTranslation: return "No translation for an address of such nature"; \
    case TC_ReportCause::noAdrTranslation: return "No translation for this specific address"; \
    case TC_ReportCause::ssnCongest: return "Subsystem congestion"; \
    case TC_ReportCause::ssnFailure: return "Subsystem failure"; \
    case TC_ReportCause::unequipUser: return "Unequipped user"; \
    case TC_ReportCause::networkFailure: return "Network failure"; \
    case TC_ReportCause::networkCongest: return "Network congestion"; \
    case TC_ReportCause::unqualified: return "Unqualified"; \
    case TC_ReportCause::hopCountViolation: return "SCCP hop counter violation"; \
    default:; } \
    return "unknown TC Report cause"; \
}

#define ODECL_RCS_TC_ReportCause() FDECL_rc2Txt_TC_ReportCause() \
URCSpacePTR  _RCS_TC_Report("errTCReport", TC_ReportCause::noAdrNatureTranslation, \
                    TC_ReportCause::hopCountViolation, rc2Txt_TC_ReportCause)


/* ------------------------------------------------------------------------- *
 * TCAP API functions(requests and indications) return/error codes.
 * NOTE: the SS7 Common parts error codes are included
 * ------------------------------------------------------------------------- */
struct TC_APIError {
    enum {
        invInvokeId = EINSS7_I97TCAP_INV_INVOKE_ID_USED,
        invDialogueId = EINSS7_I97TCAP_INV_DIALOGUE_ID,
        invQltOfSrv = EINSS7_I97TCAP_INV_QLT_OF_SERVICE,
        invDAdrLength = EINSS7_I97TCAP_INV_DEST_ADR_LENGTH,
        invOAddrLength = EINSS7_I97TCAP_INV_ORG_ADR_LENGTH,
        invTermination = EINSS7_I97TCAP_INV_TERMINATION,
        invLinkedId = EINSS7_I97TCAP_INV_LINKED_ID_USED,
        invOpClass = EINSS7_I97TCAP_INV_CLASS,
        invTimeout = EINSS7_I97TCAP_INV_TIMEOUT,
        invOpTag = EINSS7_I97TCAP_INV_OP_TAG,
        invOpLength = EINSS7_I97TCAP_INV_OP_LENGTH,
        invErrTag = EINSS7_I97TCAP_INV_ERR_TAG,
        invErrLength = EINSS7_I97TCAP_INV_ERR_LENGTH,
        invProblemTag = EINSS7_I97TCAP_INV_PROBLEM_TAG,
        invProblemCode = EINSS7_I97TCAP_INV_PROBLEM_CODE,
        invAbortInfoLength = EINSS7_I97TCAP_INV_ABORTINFO_LENGTH,
        invPrioLevel = EINSS7_I97TCAP_INV_PRIORITY_LEVEL,
        invTCUserVer = EINSS7_I97TCAP_INV_USER_VERSION,
        invACLength = EINSS7_I97TCAP_INV_AC_LENGTH,
        invUILength = EINSS7_I97TCAP_INV_UI_LENGTH,
        invUserId = EINSS7_I97TCAP_INV_USER_ID,
        indUnknownCode = EINSS7_I97TCAP_IND_UNKNOWN_CODE,
        indLength = EINSS7_I97TCAP_IND_LENGTH_ERROR
    };
};
extern const char * rc2Txt_TC_APIError(uint32_t rc_code);
extern URCSpacePTR  _RCS_TC_APIError;

#define FDECL_rc2Txt_TC_APIError()    \
const char * rc2Txt_TC_APIError(uint32_t rc_code) { \
    if (rc_code >= 1000) \
        return rc2Txt_SS7_CP(rc_code); \
    switch (rc_code) { \
    /* requests errors */ \
    case TC_APIError::invInvokeId : return "Invalid Invoke Id Used"; \
    case TC_APIError::invDialogueId : return "Invalid Dialogue Id"; \
    case TC_APIError::invQltOfSrv : return "Invalid Quality Of Service"; \
    case TC_APIError::invDAdrLength : return "Invalid Destination Address Length"; \
    case TC_APIError::invOAddrLength : return "Invalid Originating Address Length"; \
    case TC_APIError::invTermination : return "Invalid Termination"; \
    case TC_APIError::invLinkedId : return "Invalid Linked Id Used"; \
    case TC_APIError::invOpClass : return "Invalid Operation Class"; \
    case TC_APIError::invTimeout : return "Invalid Timeout"; \
    case TC_APIError::invOpTag : return "Invalid Operation Tag"; \
    case TC_APIError::invOpLength : return "Invalid Operation Length"; \
    case TC_APIError::invErrTag : return "Invalid Error Code Tag"; \
    case TC_APIError::invErrLength : return "Invalid Error Length"; \
    case TC_APIError::invProblemTag : return "Invalid Problem Code Tag"; \
    case TC_APIError::invProblemCode : return "Invalid Problem Code"; \
    case TC_APIError::invAbortInfoLength : return "Invalid Abort Info Length"; \
    case TC_APIError::invPrioLevel : return "Invalid Priority Level"; \
    case TC_APIError::invTCUserVer : return "Invalid TC-user Version"; \
    case TC_APIError::invACLength : return "Invalid Application Context Length"; \
    case TC_APIError::invUILength : return "Invalid UI Length"; \
    case TC_APIError::invUserId : return "Invalid UserId"; \
    /* indications errors */ \
    case TC_APIError::indUnknownCode : return "Unknown Code"; \
    case TC_APIError::indLength : return "Length Error"; \
    default:; } \
    return "unknown TC API error"; \
}

#define ODECL_RCS_TC_APIError() FDECL_rc2Txt_TC_APIError() \
URCSpacePTR  _RCS_TC_APIError("errTCApi", TC_APIError::invInvokeId, \
                                1510, rc2Txt_TC_APIError)


/* ------------------------------------------------------------------------- *
 * Macro for all return code spaces global initialization
 * ------------------------------------------------------------------------- */
#define _RCS_TCAPErrorsINIT()  \
    ODECL_RCS_TC_PAbortCause(); \
    ODECL_RCS_TC_UAbortCause(); \
    ODECL_RCS_TC_Bind(); \
    ODECL_RCS_TC_ReportCause(); \
    ODECL_RCS_TC_APIError()


} //inap
} //inman
} //smsc

#define _RCS_TCAPErrorsGET() \
smsc::inman::inap::_RCS_TC_PAbort.get(); \
smsc::inman::inap::_RCS_TC_UAbort.get(); \
smsc::inman::inap::_RCS_TC_Bind.get(); \
smsc::inman::inap::_RCS_TC_Report.get(); \
smsc::inman::inap::_RCS_TC_APIError.get()

#endif /* __SMSC_INMAN_INAP_TCAP_ERRORS__ */

