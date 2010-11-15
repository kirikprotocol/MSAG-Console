/* ************************************************************************* *
 * Abonent Provider query status definition and description.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_IAPRVD_ERRORS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_IAPRVD_ERRORS__

#include "util/URCdb/URCRegistry.hpp"

namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

using smsc::util::URCSpacePTR;

struct IAPQStatus {
    enum Code {
        iqOk = 0,       //query completed
        iqCancelled,    //query has been cancelled
        iqTimeout,      //query is timed out
        iqError,        //query execution error
        iqBadArg,       //invalid query argument
        iqReserved      //just a max cap
    };
};
extern const char * rc2Txt_IAPQStatus(uint32_t err_code);
extern URCSpacePTR  _RCS_IAPQStatus;

#define FDECL_rc2Txt_IAPQStatus()    \
const char * rc2Txt_IAPQStatus(uint32_t err_code) { \
    switch (err_code) { \
    case IAPQStatus::iqOk: return "query completed"; \
    case IAPQStatus::iqCancelled: return "query has been cancelled"; \
    case IAPQStatus::iqTimeout: return "query is timed out"; \
    case IAPQStatus::iqError: return "query execution error"; \
    case IAPQStatus::iqBadArg: return "invalid query argument"; \
    default:; } \
    return "unknown IAP query status"; \
}

#define ODECL_RCS_IAPQStatus() FDECL_rc2Txt_IAPQStatus() \
URCSpacePTR  _RCS_IAPQStatus("iapQueryRC", IAPQStatus::iqOk, \
                                IAPQStatus::iqReserved, rc2Txt_IAPQStatus)

/* ------------------------------------------------------------------------- *
 * Macro for all return code spaces global initialization
 * ------------------------------------------------------------------------- */
#define _RCS_IAPQStatusINIT()  ODECL_RCS_IAPQStatus()

} //iaprvd
} //inman
} //smsc

#define _RCS_IAPQStatusGET() smsc::inman::iaprvd::_RCS_IAPQStatus.get()

#endif /* __SMSC_INMAN_IAPRVD_ERRORS__ */

