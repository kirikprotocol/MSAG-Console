/* ************************************************************************* *
 * TCAP dialog error codes.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_TCDIALOG_ERR__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_TCDIALOG_ERR__

#include "util/URCdb/URCRegistry.hpp"

namespace smsc {
namespace inman {
namespace inap {

using smsc::util::URCSpacePTR;

struct TC_DlgError {
    enum Codes {
        dlgInit     = 0, //TC dialog initialization failure
        dlgParam    = 1, //invalid TC dialog parameters 
        dlgFatal    = 2, //unidentified fatal dialog error
        invCompEnc  = 3, //invalid invoke component to send
        invCompDec  = 4, //invoke component decoding failed
        resCompEnc  = 5, //invalid result/error component to send
        resCompDec  = 6  //result/error component decoding failed
    };
};
extern const char * rc2Txt_TC_Dialog(uint32_t rc_code);
extern URCSpacePTR  _RCS_TC_Dialog;

#define FDECL_rc2Txt_TC_Dialog()    \
const char * rc2Txt_TC_Dialog(uint32_t rc_code) { \
    switch (rc_code) { \
    case TC_DlgError::dlgInit: return "TC dialog initialization failure"; \
    case TC_DlgError::dlgParam: return "invalid TC dialog parameters"; \
    case TC_DlgError::dlgFatal: return "unidentified fatal dialog error"; \
    case TC_DlgError::invCompEnc: return "invalid invoke component to send"; \
    case TC_DlgError::invCompDec: return "invoke component decoding failed"; \
    case TC_DlgError::resCompEnc: return "invalid result/error component to send"; \
    case TC_DlgError::resCompDec: return "result/error component decoding failed"; \
    default:; } \
    return "unknown TC Dialog error"; \
}

#define ODECL_RCS_TC_Dialog() FDECL_rc2Txt_TC_Dialog() \
URCSpacePTR  _RCS_TC_Dialog("errTCDlg", TC_DlgError::dlgInit, \
                            TC_DlgError::resCompDec, rc2Txt_TC_Dialog)

/* ------------------------------------------------------------------------- *
 * Macro for all return code spaces global initialization
 * ------------------------------------------------------------------------- */
#define _RCS_TC_DialogINIT()  ODECL_RCS_TC_Dialog()

} //inap
} //inman
} //smsc

#define _RCS_TC_DialogGET() smsc::inman::inap::_RCS_TC_Dialog.get()
#endif /* __SMSC_INMAN_INAP_TCDIALOG_ERR__ */

