/* ************************************************************************* *
 * EINSS7 Common parts error codes descriptions(Release 5)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_SS7_MSG_ERRORS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_SS7_MSG_ERRORS__

#include "util/URCdb/URCRegistry.hpp"

#include "ss7cp.h"

namespace smsc {
namespace inman {
namespace inap {

using smsc::util::URCSpacePTR;

/* -------------------------------------------------------------------------- *
 * EINSS7 Common parts error codes descriptions(Release 5):
 * -------------------------------------------------------------------------- */
extern const char* rc2Txt_SS7_CP(uint32_t code);
extern URCSpacePTR  _RCS_SS7_Msg;

#define FDECL_rc2Txt_SS7_CP()    \
const char* rc2Txt_SS7_CP(uint32_t code) { \
    switch(code) { \
    /* Message handling */ \
    case MSG_TIMEOUT:           return "MSG TIMEOUT"; \
    case MSG_OUT_OF_MEMORY:     return "MSG OUT OF MEMORY"; \
    case MSG_ID_NOT_FOUND:      return "MSG ID NOT FOUND"; \
    case MSG_QUEUE_NOT_OPEN:    return "MSG QUEUE NOT OPEN"; \
    case MSG_NOT_OPEN:          return "MSG NOT OPEN"; \
    case MSG_SIZE_ERROR:        return "MSG SIZE ERROR"; \
    case MSG_INTERNAL_QUEUE_FULL:   return "MSG INTERNAL QUEUE FULL"; \
    case MSG_ERR:               return "MSG ERR"; \
    case MSG_SYSTEM_INTERRUPT:  return "MSG SYSTEM INTERRUPT"; \
    case MSG_NOT_CONNECTED:     return "MSG NOT CONNECTED"; \
    case MSG_NOT_ACCEPTED:      return "MSG NOT ACCEPTED"; \
    case MSG_BROKEN_CONNECTION: return "MSG BROKEN CONNECTION"; \
    case MSG_NOT_MPOWNER:       return "MSG NOT MPOWNER"; \
    case MSG_BAD_FD:            return "MSG BAD FD"; \
    case MSG_ARG_VAL:           return "MSG ARG VAL"; \
    case MSG_APPL_EVENT:        return "MSG APPL EVENT"; \
    case MSG_OPEN_FAILED:       return "MSG OPEN FAILED"; \
    case MSG_WOULD_BLOCK:       return "MSG WOULD BLOCK"; \
    case EINSS7CP_MSG_GETBUF_FAIL:  return "MSG GETBUF FAIL"; \
    case EINSS7CP_MSG_RELBUF_FAIL:  return "MSG_RELBUF_FAIL"; \
    case MSG_INSTANCE_NOT_FOUND:    return "MSG INSTANCE NOT FOUND"; \
    /* Timer handling */ \
    case TIME_INIT_FAILED:      return "TIME INIT FAILED"; \
    case TOO_MANY_TIMERS:       return "TOO MANY TIMERS"; \
    /* Interrupt handling */ \
    case EINSS7CP_NAME_NOT_FOUND:   return "NAME NOT FOUND"; \
    case MSG_ID_NOT_IN_CONFIG_FILE: return "MSG ID NOT IN CONFIG FILE"; \
    /* Xtended memory handling */ \
    case XMEM_INVALID_USERID: return "XMEM INVALID USERID"; \
    case XMEM_OUT_OF_MEMORY: return "XMEM OUT OF MEMORY"; \
    case XMEM_INVALID_POINTER: return "XMEM INVALID POINTER"; \
    case XMEM_INIT_DONE: return "XMEM INIT DONE"; \
    case XMEM_INVALID_PARAMETER: return "XMEM INVALID PARAMETER"; \
    case XMEM_INVALID_SIZE: return "XMEM INVALID SIZE"; \
    case XMEM_CORRUPT_MEM: return "XMEM CORRUPT MEM"; \
    /* Thread problems */ \
    case EINSS7CP_THREAD_MI_PROBLEM: return "THREAD MI PROBLEM"; \
    case EINSS7CP_THREAD_KC_PROBLEM: return "THREAD KC PROBLEM"; \
    case EINSS7CP_THREAD_CI_PROBLEM: return "THREAD CI PROBLEM"; \
    case EINSS7CP_MPOWNER_CLOSED: return "MPOWNER CLOSED"; \
    case EINSS7CP_TIME_NOT_INIT: return "TIME NOT INIT"; \
    case EINSS7CP_NO_TOKEN: return "NO TOKEN"; \
    case EINSS7CP_NO_THREAD_CREATED/*1137*/: return "NO THREAD CREATED"; \
    case EINSS7CP_ANOTHER_THREAD_CLOSING_CON/*1143*/: return "ANOTHER THREAD CLOSING CON"; \
    case EINSS7CP_NO_IPC_EXIST/*1145*/: return "NO IPC EXIST"; \
    case EINSS7CP_CREATE_IPC_ERROR/*1146*/: return "CREATE IPC ERROR"; \
    /* Errors in socket and event handling */ \
    case EINSS7CP_INVALID_SOCKET: return "INVALID SOCKET"; \
    case EINSS7CP_SOCKET_ERROR: return "SOCKET ERROR"; \
    case EINSS7CP_INVALID_EVENT: return "INVALID EVENT"; \
    case EINSS7CP_EVENT_ERROR: return "EVENT ERROR"; \
    /* Errors in Message Pool */ \
    case EINSS7CP_MSG_POOL_ALREADY_CREATED: return "MSG POOL ALREADY CREATED"; \
    case EINSS7CP_MSG_POOL_MAX_NUMBER_REACHED: return "MSG POOL MAX NUMBER REACHED"; \
    case EINSS7CP_INVALID_POINTER: return "INVALID POINTER"; \
    case EINSS7CP_MSG_MSGDELAY_ERROR: return "MSG MSGDELAY ERROR"; \
    /* Errors in config file */ \
    case MSG_UNKNOWN_FILE: return "MSG UNKNOWN FILE"; \
    case MSG_IPA_UNKNOWN_MODULE: return "MSG IPA UNKNOWN MODULE"; \
    case MSG_IPA_SYNTAX_ERROR: return "MSG IPA SYNTAX ERROR"; \
    case MSG_IPA_NOT_MPOWNER: return "MSG IPA NOT MPOWNER"; \
    case MSG_IPA_MISSING_ADDRESS: return "MSG IPA MISSING ADDRESS"; \
    case MSG_IPA_MULTIPLE_ENTRIES: return "MSG IPA MULTIPLE ENTRIES"; \
    case MSG_INTERACT_UNKNOWN_MODULE: return "MSG INTERACT UNKNOWN MODULE"; \
    case MSG_INTERACT_SYNTAX_ERROR: return "MSG INTERACT SYNTAX ERROR"; \
    case MSG_INTERACT_MULTIPLE_ENTRIES: return "MSG INTERACT MULTIPLE ENTRIES"; \
    case MSG_INTERACT_NOT_MPOWNER: return "MSG INTERACT NOT MPOWNER"; \
    case MSG_BUFSIZE_ERROR: return "MSG BUFSIZE ERROR"; \
    case MSG_MAXENTRIES_ERROR: return "MSG MAXENTRIES ERROR"; \
    case MSG_CONFIG_ERR: return "MSG CONFIG ERR"; \
    case LOG_WRITEMODE_ERROR: return "LOG WRITEMODE ERROR"; \
    case LOG_FILESIZE_ERROR: return "LOG FILESIZE ERROR"; \
    case LOG_FILEPATH_ERROR: return "LOG FILEPATH ERROR"; \
    case LOG_INTERNALBUFFER_ERROR: return "LOG INTERNALBUFFER ERROR"; \
    case LOG_FILEFORMAT_ERROR: return "LOG FILEFORMAT ERROR"; \
    case MSG_HBLOST_ERROR: return "MSG HBLOST ERROR"; \
    case MSG_HBRATE_ERROR: return "MSG HBRATE ERROR"; \
    case MSG_MSGNONBLOCK_ERROR: return "MSG MSGNONBLOCK ERROR"; \
    case EINSS7CP_MSG_SYSTIME_ERROR: return "MSG SYSTIME ERROR"; \
    case EINSS7CP_LOG_PROCPATH_ERROR: return "LOG PROCPATH ERROR"; \
    case EINSS7CP_LOG_FILEPROP_ERROR: return "LOG FILEPROP ERROR"; \
    /* GVR: the ss7tmc.h has a conflict: */ \
    /* MSG_HBOFF_UNKNOWN_MODULE has the same value as EINSS7CP_MSG_MSGDELAY_ERROR */ \
    /* case MSG_HBOFF_UNKNOWN_MODULE: return "MSG HBOFF UNKNOWN MODULE"; \ */ \
    case MSG_HBOFF_NOT_MPOWNER: return "MSG HBOFF NOT MPOWNER"; \
    case MSG_HBOFF_SYNTAX_ERROR: return "MSG HBOFF SYNTAX ERROR"; \
    case EINSS7CP_MSG_UXDOM_SYNTAX_ERROR: return "MSG UXDOM SYNTAX ERROR"; \
    case EINSS7CP_MSG_UXDOM_NOT_AVAILABLE: return "MSG UXDOM NOT AVAILABLE"; \
    case EINSS7CP_MSG_UXDOM_ARG_ERROR: return "MSG UXDOM ARG ERROR"; \
    case EINSS7CP_LOG_BACKFILEPROP_ERROR: return "LOG BACKFILEPROP ERROR"; \
    case EINSS7CP_LOG_LD_IPA_ERROR: return "LOG LD IPA ERROR"; \
    case EINSS7CP_LOG_FLUSH_ERROR: return "LOG FLUSH ERROR"; \
    case EINSS7CP_LOGD_SOCKET_ERROR: return "LOGD SOCKET ERROR"; \
    case EINSS7CP_LOGD_BIND_ERROR: return "LOGD BIND ERROR"; \
    case EINSS7CP_LOG_NOT_INITIATED: return "LOG NOT INITIATED"; \
    case EINSS7CP_MSGINIT_DONE: return "MSGINIT DONE"; \
    case EINSS7CP_STOP_SOCKET_ERROR: return "STOP SOCKET ERROR"; \
    case EINSS7CP_SIGNAL_ERROR: return "SIGNAL ERROR"; \
    case EINSS7CP_MSG_NO_BUFFER: return "MSG NO BUFFER"; \
    case EINSS7CP_MUTEX_INIT_FAILED: return "MUTEX INIT FAILED"; \
    case EINSS7CP_COND_INIT_FAILED: return "COND INIT FAILED"; \
    /* */ \
    case EINSS7CP_AUTOSETUP_ADDR: return "AUTOSETUP ADDR"; \
    case EINSS7_EMPTY_BUFFER: return "EMPTY BUFFER"; \
    case EINSS7CP_READ_SOCKET_BUFFER_ERROR: return "READ SOCKET BUFFER ERROR"; \
    case EINSS7CP_WRITE_SOCKET_BUFFER_ERROR: return "WRITE SOCKET BUFFER ERROR"; \
    case EINSS7CP_SEND_BUFFER_ERROR: return "SEND BUFFER ERROR"; \
    case EINSS7CP_MSG_SEND_FAIL: return "MSG SEND FAIL"; \
    case EINSS7CP_MSGINIT_NOT_DONE: return "MSGINIT NOT DONE"; \
    case EINSS7CP_NOT_IMPLEMENTED: return "NOT IMPLEMENTED"; \
    case EINSS7CP_TIMER_NOT_IN_USE: return "TIMER NOT IN USE"; \
    case MSG_CONNTYPE_NOT_MPOWNER: return "MSG CONNTYPE NOT MPOWNER"; \
    case LOGALARM_SYNTAX_ERROR: return "LOGALARM SYNTAX ERROR"; \
    case EINSS7CP_UNIXSOCKPATH_ERROR: return "UNIX SOCKPATH ERROR"; \
    case EINSS7CP_MSGNODELAY_ERROR: return "MSGN ODELAY ERROR"; \
    case EINSS7CP_MSGTRACEON_ERROR: return "MSG TRACEON ERROR"; \
    case EINSS7CP_MSGCONNTYPE_ERROR: return "MSG CONNTYPE ERROR"; \
    case EINSS7CP_TESTMODULE_ERROR: return "TESTMODULE ERROR"; \
    case EINSS7CP_NO_CPMANAGER_ERROR: return "NO CPMANAGER ERROR"; \
    case EINSS7CP_CONNTIMEOUT_ERROR: return "CONN TIMEOUT ERROR"; \
    case EINSS7CP_SOCKETTIMEOUT_ERROR: return "SOCKET TIMEOUT ERROR"; \
    case FILEACCESSTYPE_ERROR: return "FILE ACCESS TYPE ERROR"; \
    case EINSS7CP_LOGSPERSECOND_ERROR: return "LOGS PER SECOND ERROR"; \
    case EINSS7CP_LOGSPERSECOND_MAX_REACHED: return "LOGS PER SECOND MAX REACHED"; \
    /* Callback error codes */ \
    case EINSS7CP_MSGREGBROKENCONNFAIL: return "MSG REG BROKEN CONN FAIL"; \
    case EINSS7CP_MSGREGISTERCALLBACKFAIL: return "MSG REGISTER CALLBACK FAIL"; \
    /* vxworks error codes */ \
    case MSG_ZBUF_CREATE_FAIL/*1400*/: return "MSG ZBUF CREATE FAIL"; \
    case MSG_ZBUF_DELETE_FAIL/*1401*/: return "MSG ZBUF DELETE FAIL"; \
    case MSG_ZBUF_SEND_FAIL/*1402*/: return "MSG ZBUF SEND FAIL"; \
    /* cp mgmt error codes */ \
    case EINSS7CP_CPMGMT_NOT_USED/*1450*/: return "CPMGMT NOT USED"; \
    /* OSE Error codes */ \
    case MSG_OSE_SYNTAX_ERROR: return "MSG OSE SYNTAX ERROR"; \
    case ERROR_DESTINATION_UNKNOWN: return "ERROR DESTINATION UNKNOWN"; \
    case MSG_APPL_EVENT_EXTERNAL: return "MSG APPL EVENT EXTERNAL"; \
    case EINSS7CP_CELLO_BAD_IPADDR: return "CELLO BAD IPADDR"; \
    case EINSS7CP_CELLO_UNSUPP_FAMILY: return "CELLO UNSUPP FAMILY"; \
    case EINSS7CP_EXTSIGREGISTERCALLBACKFAIL: return "EXTSIGREGISTERCALLBACKFAIL"; \
    case MSG_OSE_UNKNOWN_MODULE /*1506*/: return "MSG OSE UNKNOWN MODULE"; \
    default:; } \
    return "UNKNOWN CODE"; \
}

#define ODECL_RCS_SS7_CP() FDECL_rc2Txt_SS7_CP() \
URCSpacePTR  _RCS_SS7_Msg("errSS7Msg", 1000, 1510, rc2Txt_SS7_CP)


/* ------------------------------------------------------------------------- *
 * Macro for all return code spaces global initialization
 * ------------------------------------------------------------------------- */
#define _RCS_SS7_CP_INIT()  ODECL_RCS_SS7_CP()

} //inap
} //inman
} //smsc

#define _RCS_SS7_CP_GET() smsc::inman::inap::_RCS_SS7_Msg.get()
#endif /* __SMSC_INMAN_INAP_SS7_MSG_ERRORS__ */

