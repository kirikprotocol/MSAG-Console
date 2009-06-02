#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "i97tcapapi.h"

#include "einhd_test.hpp"
#include "einhd_utl.hpp"

const char* rc2Txt_SS7_CP(uint32_t code) {
    switch(code) {
    /* Message handling */
    case MSG_TIMEOUT:           return "MSG TIMEOUT";
    case MSG_OUT_OF_MEMORY:     return "MSG OUT OF MEMORY";
    case MSG_ID_NOT_FOUND:      return "MSG ID NOT FOUND";
    case MSG_QUEUE_NOT_OPEN:    return "MSG QUEUE NOT OPEN";
    case MSG_NOT_OPEN:          return "MSG NOT OPEN";
    case MSG_SIZE_ERROR:        return "MSG SIZE ERROR";
    case MSG_INTERNAL_QUEUE_FULL:   return "MSG INTERNAL QUEUE FULL";
    case MSG_ERR:               return "MSG ERR";
    case MSG_SYSTEM_INTERRUPT:  return "MSG SYSTEM INTERRUPT";
    case MSG_NOT_CONNECTED:     return "MSG NOT CONNECTED";
    case MSG_NOT_ACCEPTED:      return "MSG NOT ACCEPTED";
    case MSG_BROKEN_CONNECTION: return "MSG BROKEN CONNECTION";
    case MSG_NOT_MPOWNER:       return "MSG NOT MPOWNER";
    case MSG_BAD_FD:            return "MSG BAD FD";
    case MSG_ARG_VAL:           return "MSG ARG VAL";
    case MSG_APPL_EVENT:        return "MSG APPL EVENT";
    case MSG_OPEN_FAILED:       return "MSG OPEN FAILED";
    case MSG_WOULD_BLOCK:       return "MSG WOULD BLOCK";
    case EINSS7CP_MSG_GETBUF_FAIL:  return "MSG GETBUF FAIL";
    case EINSS7CP_MSG_RELBUF_FAIL:  return "MSG_RELBUF_FAIL";
    case MSG_INSTANCE_NOT_FOUND:    return "MSG INSTANCE NOT FOUND";
    /* Timer handling */
    case TIME_INIT_FAILED:      return "TIME INIT FAILED";
    case TOO_MANY_TIMERS:       return "TOO MANY TIMERS";
    /* Interrupt handling */
    case EINSS7CP_NAME_NOT_FOUND:   return "NAME NOT FOUND";
    case MSG_ID_NOT_IN_CONFIG_FILE: return "MSG ID NOT IN CONFIG FILE";
    /* Xtended memory handling */
    case XMEM_INVALID_USERID: return "XMEM INVALID USERID";
    case XMEM_OUT_OF_MEMORY: return "XMEM OUT OF MEMORY";
    case XMEM_INVALID_POINTER: return "XMEM INVALID POINTER";
    case XMEM_INIT_DONE: return "XMEM INIT DONE";
    case XMEM_INVALID_PARAMETER: return "XMEM INVALID PARAMETER";
    case XMEM_INVALID_SIZE: return "XMEM INVALID SIZE";
    case XMEM_CORRUPT_MEM: return "XMEM CORRUPT MEM";
    /* Thread problems */
    case EINSS7CP_THREAD_MI_PROBLEM: return "THREAD MI PROBLEM";
    case EINSS7CP_THREAD_KC_PROBLEM: return "THREAD KC PROBLEM";
    case EINSS7CP_THREAD_CI_PROBLEM: return "THREAD CI PROBLEM";
    case EINSS7CP_MPOWNER_CLOSED: return "MPOWNER CLOSED";
    case EINSS7CP_TIME_NOT_INIT: return "TIME NOT INIT";
    case EINSS7CP_NO_TOKEN: return "NO TOKEN";
    case EINSS7CP_NO_THREAD_CREATED/*1137*/: return "NO THREAD CREATED";
    case EINSS7CP_ANOTHER_THREAD_CLOSING_CON/*1143*/: return "ANOTHER THREAD CLOSING CON";
    case EINSS7CP_NO_IPC_EXIST/*1145*/: return "NO IPC EXIST";
    case EINSS7CP_CREATE_IPC_ERROR/*1146*/: return "CREATE IPC ERROR";
    /* Errors in socket and event handling */
    case EINSS7CP_INVALID_SOCKET: return "INVALID SOCKET";
    case EINSS7CP_SOCKET_ERROR: return "SOCKET ERROR";
    case EINSS7CP_INVALID_EVENT: return "INVALID EVENT";
    case EINSS7CP_EVENT_ERROR: return "EVENT ERROR";
    /* Errors in Message Pool */
    case EINSS7CP_MSG_POOL_ALREADY_CREATED: return "MSG POOL ALREADY CREATED";
    case EINSS7CP_MSG_POOL_MAX_NUMBER_REACHED: return "MSG POOL MAX NUMBER REACHED";
    case EINSS7CP_INVALID_POINTER: return "INVALID POINTER";
    case EINSS7CP_MSG_MSGDELAY_ERROR: return "MSG MSGDELAY ERROR";
    /* Errors in config file */
    case MSG_UNKNOWN_FILE: return "MSG UNKNOWN FILE";
    case MSG_IPA_UNKNOWN_MODULE: return "MSG IPA UNKNOWN MODULE";
    case MSG_IPA_SYNTAX_ERROR: return "MSG IPA SYNTAX ERROR";
    case MSG_IPA_NOT_MPOWNER: return "MSG IPA NOT MPOWNER";
    case MSG_IPA_MISSING_ADDRESS: return "MSG IPA MISSING ADDRESS";
    case MSG_IPA_MULTIPLE_ENTRIES: return "MSG IPA MULTIPLE ENTRIES";
    case MSG_INTERACT_UNKNOWN_MODULE: return "MSG INTERACT UNKNOWN MODULE";
    case MSG_INTERACT_SYNTAX_ERROR: return "MSG INTERACT SYNTAX ERROR";
    case MSG_INTERACT_MULTIPLE_ENTRIES: return "MSG INTERACT MULTIPLE ENTRIES";
    case MSG_INTERACT_NOT_MPOWNER: return "MSG INTERACT NOT MPOWNER";
    case MSG_BUFSIZE_ERROR: return "MSG BUFSIZE ERROR";
    case MSG_MAXENTRIES_ERROR: return "MSG MAXENTRIES ERROR";
    case MSG_CONFIG_ERR: return "MSG CONFIG ERR";
    case LOG_WRITEMODE_ERROR: return "LOG WRITEMODE ERROR";
    case LOG_FILESIZE_ERROR: return "LOG FILESIZE ERROR";
    case LOG_FILEPATH_ERROR: return "LOG FILEPATH ERROR";
    case LOG_INTERNALBUFFER_ERROR: return "LOG INTERNALBUFFER ERROR";
    case LOG_FILEFORMAT_ERROR: return "LOG FILEFORMAT ERROR";
    case MSG_HBLOST_ERROR: return "MSG HBLOST ERROR";
    case MSG_HBRATE_ERROR: return "MSG HBRATE ERROR";
    case MSG_MSGNONBLOCK_ERROR: return "MSG MSGNONBLOCK ERROR";
    case EINSS7CP_MSG_SYSTIME_ERROR: return "MSG SYSTIME ERROR";
    case EINSS7CP_LOG_PROCPATH_ERROR: return "LOG PROCPATH ERROR";
    case EINSS7CP_LOG_FILEPROP_ERROR: return "LOG FILEPROP ERROR";
    /* NOTE: the ss7tmc.h has a conflict: */
    /* MSG_HBOFF_UNKNOWN_MODULE has the same value as EINSS7CP_MSG_MSGDELAY_ERROR */
    //case MSG_HBOFF_UNKNOWN_MODULE: return "MSG HBOFF UNKNOWN MODULE";
    case MSG_HBOFF_NOT_MPOWNER: return "MSG HBOFF NOT MPOWNER";
    case MSG_HBOFF_SYNTAX_ERROR: return "MSG HBOFF SYNTAX ERROR";
    case EINSS7CP_MSG_UXDOM_SYNTAX_ERROR: return "MSG UXDOM SYNTAX ERROR";
    case EINSS7CP_MSG_UXDOM_NOT_AVAILABLE: return "MSG UXDOM NOT AVAILABLE";
    case EINSS7CP_MSG_UXDOM_ARG_ERROR: return "MSG UXDOM ARG ERROR";
    case EINSS7CP_LOG_BACKFILEPROP_ERROR: return "LOG BACKFILEPROP ERROR";
    case EINSS7CP_LOG_LD_IPA_ERROR: return "LOG LD IPA ERROR";
    case EINSS7CP_LOG_FLUSH_ERROR: return "LOG FLUSH ERROR";
    case EINSS7CP_LOGD_SOCKET_ERROR: return "LOGD SOCKET ERROR";
    case EINSS7CP_LOGD_BIND_ERROR: return "LOGD BIND ERROR";
    case EINSS7CP_LOG_NOT_INITIATED: return "LOG NOT INITIATED";
    case EINSS7CP_MSGINIT_DONE: return "MSGINIT DONE";
    case EINSS7CP_STOP_SOCKET_ERROR: return "STOP SOCKET ERROR";
    case EINSS7CP_SIGNAL_ERROR: return "SIGNAL ERROR";
    case EINSS7CP_MSG_NO_BUFFER: return "MSG NO BUFFER";
    case EINSS7CP_MUTEX_INIT_FAILED: return "MUTEX INIT FAILED";
    case EINSS7CP_COND_INIT_FAILED: return "COND INIT FAILED";
    /* */
    case EINSS7CP_AUTOSETUP_ADDR: return "AUTOSETUP ADDR";
    case EINSS7_EMPTY_BUFFER: return "EMPTY BUFFER";
    case EINSS7CP_READ_SOCKET_BUFFER_ERROR: return "READ SOCKET BUFFER ERROR";
    case EINSS7CP_WRITE_SOCKET_BUFFER_ERROR: return "WRITE SOCKET BUFFER ERROR";
    case EINSS7CP_SEND_BUFFER_ERROR: return "SEND BUFFER ERROR";
    case EINSS7CP_MSG_SEND_FAIL: return "MSG SEND FAIL";
    case EINSS7CP_MSGINIT_NOT_DONE: return "MSGINIT NOT DONE";
    case EINSS7CP_NOT_IMPLEMENTED: return "NOT IMPLEMENTED";
    case EINSS7CP_TIMER_NOT_IN_USE: return "TIMER NOT IN USE";
    case MSG_CONNTYPE_NOT_MPOWNER: return "MSG CONNTYPE NOT MPOWNER";
    case LOGALARM_SYNTAX_ERROR: return "LOGALARM SYNTAX ERROR";
    case EINSS7CP_UNIXSOCKPATH_ERROR: return "UNIX SOCKPATH ERROR";
    case EINSS7CP_MSGNODELAY_ERROR: return "MSGN ODELAY ERROR";
    case EINSS7CP_MSGTRACEON_ERROR: return "MSG TRACEON ERROR";
    case EINSS7CP_MSGCONNTYPE_ERROR: return "MSG CONNTYPE ERROR";
    case EINSS7CP_TESTMODULE_ERROR: return "TESTMODULE ERROR";
    case EINSS7CP_NO_CPMANAGER_ERROR: return "NO CPMANAGER ERROR";
    case EINSS7CP_CONNTIMEOUT_ERROR: return "CONN TIMEOUT ERROR";
    case EINSS7CP_SOCKETTIMEOUT_ERROR: return "SOCKET TIMEOUT ERROR";
    case FILEACCESSTYPE_ERROR: return "FILE ACCESS TYPE ERROR";
    case EINSS7CP_LOGSPERSECOND_ERROR: return "LOGS PER SECOND ERROR";
    case EINSS7CP_LOGSPERSECOND_MAX_REACHED: return "LOGS PER SECOND MAX REACHED";
    /* Callback error codes */
    case EINSS7CP_MSGREGBROKENCONNFAIL: return "MSG REG BROKEN CONN FAIL";
    case EINSS7CP_MSGREGISTERCALLBACKFAIL: return "MSG REGISTER CALLBACK FAIL";
    /* vxworks error codes */
    case MSG_ZBUF_CREATE_FAIL/*1400*/: return "MSG ZBUF CREATE FAIL";
    case MSG_ZBUF_DELETE_FAIL/*1401*/: return "MSG ZBUF DELETE FAIL";
    case MSG_ZBUF_SEND_FAIL/*1402*/: return "MSG ZBUF SEND FAIL";
    /* cp mgmt error codes */
    case EINSS7CP_CPMGMT_NOT_USED/*1450*/: return "CPMGMT NOT USED";
    /* OSE Error codes */
    case MSG_OSE_SYNTAX_ERROR: return "MSG OSE SYNTAX ERROR";
    case ERROR_DESTINATION_UNKNOWN: return "ERROR DESTINATION UNKNOWN";
    case MSG_APPL_EVENT_EXTERNAL: return "MSG APPL EVENT EXTERNAL";
    case EINSS7CP_CELLO_BAD_IPADDR: return "CELLO BAD IPADDR";
    case EINSS7CP_CELLO_UNSUPP_FAMILY: return "CELLO UNSUPP FAMILY";
    case EINSS7CP_EXTSIGREGISTERCALLBACKFAIL: return "EXTSIGREGISTERCALLBACKFAIL";
    case MSG_OSE_UNKNOWN_MODULE /*1506*/: return "MSG OSE UNKNOWN MODULE";
    default:; }
    return "UNKNOWN CODE";
}

/* ************************************************************************** *
 * class TCAPConnector implementation:
 * ************************************************************************** */
TCAPConnector::TCAPConnector()
    : _ss7State(ss7None)
#ifdef EIN_HD
    , _rcpMgrAdr(NULL)
#endif /* EIN_HD */
    , _logId("TCCon"), logger(NULL)
{ }

TCAPConnector::~TCAPConnector()
{
  if (logger) {
      disconnectCP(ss7None);
  #ifdef EIN_HD
      delete [] _rcpMgrAdr;
  #endif /* EIN_HD */
      smsc_log_info(logger, "%s: SS7 stack disconnected", _logId);
  }
}
    
void TCAPConnector::Init(const TCAPHD_CFG & use_cfg, Logger * use_log/* = NULL*/)
{
  _cfg = use_cfg;
  _cfg.mpUserId += USER01_ID - 1; //adjust USER_ID to PortSS7 units id
#ifdef EIN_HD
  _rcpMgrAdr = new char[_cfg.rcpMgrAdr.length() + 2];
  strcpy(_rcpMgrAdr, _cfg.rcpMgrAdr.c_str());
#else EIN_HD
  //set the single TCAP BE instance with id = 0
  _cfg.instIds.addInstance(0);
#endif /* EIN_HD*/

#ifdef USE_PRIVATE_LOGGER
  logger = use_log ? use_log : Logger::getInstance(LOGGER_CATEGORY);
#else
  logger = use_log ? use_log : stdout;
#endif /* USE_PRIVATE_LOGGER */
}


// -------------------------------------------------------------------------
// TCAP messages(indications) listener methods
// -------------------------------------------------------------------------

void TCAPConnector::onDisconnect(unsigned char inst_id)
{
  SS7UnitInstance * pInst = _cfg.instIds.getInstance(inst_id);
  if (pInst && (pInst->connStatus == SS7UnitInstance::uconnOk)) {
    pInst->connStatus = SS7UnitInstance::uconnError;
    smsc_log_error(logger, "%s: connection broken userId=%u -> TCAP[instId = %u]",
                    _logId, _cfg.mpUserId, inst_id);
  } else
    smsc_log_debug(logger, "%s: connection broken userId=%u -> TCAP[instId = %u]",
                  _logId, _cfg.mpUserId, inst_id);
}

void TCAPConnector::onDisconnect(unsigned short from_usrID,
                         unsigned short to_usrID, unsigned char inst_id)
{
  if ((from_usrID != _cfg.mpUserId) || (to_usrID != TCAP_ID)) {
    smsc_log_warn(logger, "%s: connection broken userId=%u -> userId=%u[instId = %u]",
                   _logId, from_usrID, to_usrID, inst_id);
    return;
  }
  onDisconnect(inst_id);
}

/* -------------------------------------------------------------------------- *
 * PRIVATE:
 * -------------------------------------------------------------------------- */
//Checks for unconnected TCAP BE instances and returns its total number
unsigned TCAPConnector::disconnectedUnits(bool * is_all/* = NULL*/) const
{
    unsigned rval = 0, i = 0;
    for (SS7UnitInstsMap::const_iterator it = _cfg.instIds.begin();
                                    it != _cfg.instIds.end(); ++it, ++i) {
      if (it->second.connStatus != SS7UnitInstance::uconnOk)
        ++rval;
    }
    if (is_all)
      *is_all = (rval == i);
//smsc_log_debug(logger, "%s: disconnectedUnits(): %u {%s}", _logId, rval, rval == i ? "ALL" : " ");
    return rval;
}

//Connects currently disconnected TCAP BE instances.
//Returns number of new instances succesfully connected
unsigned TCAPConnector::connectUnits(void)
{
  unsigned cnt = 0;
  for (SS7UnitInstsMap::iterator it = _cfg.instIds.begin(); 
                                it != _cfg.instIds.end(); ++it) {
//    smsc_log_debug(logger, "%s: connectUnits() unit[%u] status: %u", _logId,
//                   it->second.instId, it->second.connStatus);
    if (it->second.connStatus >= SS7UnitInstance::uconnAwaited)
      continue;
    it->second.connStatus = SS7UnitInstance::uconnAwaited;
    USHORT_T result = EINSS7CpMsgConnNotify(_cfg.mpUserId, TCAP_ID, it->second.instId,
                                            onEINSS7CpConnectBroken);
    if (result != 0) {
      smsc_log_error(logger, "%s: MsgConn(TCAP instId = %u) failed: %s (code %u)",
                  _logId, (unsigned)it->second.instId, rc2Txt_SS7_CP(result), result);
      it->second.connStatus = SS7UnitInstance::uconnError;
    } else {
      smsc_log_info(logger, "%s: MsgConn(TCAP instId = %u) Ok!",
                  _logId, (unsigned)it->second.instId);
      it->second.connStatus = SS7UnitInstance::uconnOk;
      ++cnt;
    }
  }
//  smsc_log_debug(logger, "%s: connectUnits() {%u}", _logId, cnt);
  return cnt;
}

//Disconnects all TCAP BE instances.
void TCAPConnector::disconnectUnits(void)
{
  SS7UnitInstsMap::iterator it = _cfg.instIds.begin();
  for (; it != _cfg.instIds.end(); ++it) {
    if (it->second.connStatus == SS7UnitInstance::uconnOk) {
      USHORT_T result = EINSS7CpMsgRelInst(_cfg.mpUserId, TCAP_ID, it->second.instId);
      if (result) 
          smsc_log_error(logger, "%s: MsgRel(TCAP instId = %u) failed: %s (code %u)",
                      _logId, (unsigned)it->second.instId, rc2Txt_SS7_CP(result), result);
      else
          smsc_log_debug(logger, "%s: MsgRel(TCAP instId = %u)",
                      _logId, (unsigned)it->second.instId);
          
    }
    it->second.connStatus = SS7UnitInstance::uconnIdle;
  }
  return;
}


//Returns:  (-1) - failed to connect, 0 - already connected, 1 - successfully connected
int TCAPConnector::connectCP(SS7State_e upTo/* = ss7CONNECTED*/)
{
    if (_ss7State >= upTo)
      return 0;
    smsc_log_info(logger, "%s: connecting SS7 stack: %u -> %u ..", _logId, _ss7State, upTo);
    
    USHORT_T result;
    int    rval = 1;
    while ((rval > 0) && (_ss7State < upTo)) {
      switch (_ss7State) {
      case ss7None: {
#ifdef EIN_HD
        EINSS7CpMain_CpInit();
#endif /* EIN_HD */
        _ss7State = ss7INITED;
        smsc_log_info(logger, "%s: state INITED", _logId);
      } break;

      case ss7INITED: {
        //Initiation of the message handling and allocating memory,
        //reading cp.cnf, connecting to CP manager
#ifdef EIN_HD
        result = EINSS7CpRegisterMPOwner(_cfg.mpUserId);
        if (result != RETURN_OK) {
          smsc_log_fatal(logger, "%s: CpRegisterMPOwner(userId = %u)"
                                 " failed: %s (code %u)", _logId,
                          (unsigned)_cfg.mpUserId, rc2Txt_SS7_CP(result), result);
          rval = -1;
          break;
        }
        result = EINSS7CpRegisterRemoteCPMgmt(CP_MANAGER_ID, _cfg.rcpMgrInstId, _rcpMgrAdr);
        if (result != RETURN_OK) {
          smsc_log_fatal(logger, "%s: CpRegisterRemoteCPMgmt() failed: %s (code %u)", _logId,
                          rc2Txt_SS7_CP(result), result);
          rval = -1;
          break;
        }
#endif /* EIN_HD */
        result = EINSS7CpMsgInitiate(_cfg.maxMsgNum, _cfg.appInstId, 0);
        if (result != 0) {
          smsc_log_fatal(logger, "%s: CpMsgInitiate(appInstanceId = %u)"
                                 " failed: %s (code %u)", _logId,
                         (unsigned)_cfg.appInstId, rc2Txt_SS7_CP(result), result);
          rval = -1;
          break;
        }
        _ss7State = ss7REGISTERED;
#ifdef EIN_HD
        smsc_log_info(logger, "%s: state REGISTERED (appInstanceId = %u,"
                              " userId = %u, CpMgr: %s)",
                      _logId, (unsigned)_cfg.appInstId, (unsigned)_cfg.mpUserId,
                      _cfg.rcpMgrAdr.c_str());
#else  /* EIN_HD */
        smsc_log_info(logger, "%s: state REGISTERED (appInstanceId = %u, userId = %u)",
                      _logId, (unsigned)_cfg.appInstId, (unsigned)_cfg.mpUserId);
#endif /* EIN_HD */
      } break;

      case ss7REGISTERED: {  //Opening of the input queue.
        result = EINSS7CpMsgPortOpen(_cfg.mpUserId, TRUE);
        if (result != 0) {
            smsc_log_fatal(logger, "%s: CpMsgPortOpen(userId = %u) failed: %s (code %u)",
                           _logId, _cfg.mpUserId, rc2Txt_SS7_CP(result), result);
            rval = -1;
            break;
        }
        _ss7State = ss7OPENED;
        smsc_log_info(logger, "%s: state OPENED (userId = %u)", _logId, _cfg.mpUserId);
      } break;

      case ss7OPENED: { //Connecting user to TCAP unit input queue.
        if (!connectUnits()) {
          smsc_log_fatal(logger, "%s: MsgConn(TCAP all instances) failed", _logId);
          rval = -1;
          break;
        }
        //set CONNECTED state if at least one instance is succesfully connected
        _ss7State = ss7CONNECTED;
        smsc_log_info(logger, "%s: state CONNECTED (userId = %u)", _logId, _cfg.mpUserId);
      } break;

      default:;
      } /* eosw */
    }
    return rval;
}
//
void TCAPConnector::disconnectCP(SS7State_e downTo/* = ss7None*/)
{
    if (_ss7State > downTo)
        smsc_log_info(logger, "%s: disconnecting SS7 stack: %u -> %u ..", _logId, _ss7State, downTo);
    while (_ss7State > downTo) {
      switch (_ss7State) {
      case ss7CONNECTED: { //Releasing of connections to other users.
        disconnectUnits();
        smsc_log_info(logger, "%s: state OPENED", _logId);
        _ss7State = ss7OPENED;
      } break;

      case ss7OPENED: { //Closing of the input queue.
        USHORT_T result;
        if ((result = MsgClose(_cfg.mpUserId)) != 0)
            smsc_log_error(logger, "%s: MsgClose(userId = %u) failed: %s (code %u)",
                           _logId, (unsigned)_cfg.mpUserId, 
                           rc2Txt_SS7_CP(result), result);
        smsc_log_info(logger, "%s: state REGISTERED", _logId);
        _ss7State = ss7REGISTERED;
      } break;

      case ss7REGISTERED: {
        MsgExit();
        _ss7State = ss7INITED;
        smsc_log_info(logger, "%s: state INITED", _logId);
      } break;

      case ss7INITED: {
        _ss7State = ss7None;
        smsc_log_info(logger, "%s: state NONE", _logId);
      } break;
      //case ss7None:
      default:;
      } /* eosw */
    }
    return;
}


/* ************************************************************************** *
 * Utility functions
 * ************************************************************************** */
#include <stdarg.h>
unsigned vpformat(const char* fmt, va_list arg_list, char ** p_buf,
                 unsigned buf_sz/* = 1024*/)
{
    char * vbuf = new char[buf_sz];
    int n = vsnprintf(vbuf, buf_sz - 1, fmt, arg_list);
    if (n >= buf_sz) {
        buf_sz = n + 2; // + '\0'
        delete [] vbuf;
        vbuf = new char[buf_sz];
        n = vsnprintf(vbuf, buf_sz - 1, fmt, arg_list);
    }
    vbuf[(n >= 0) ? n : 0] = 0; //vsnprintf() may return -1 on error
    *p_buf = vbuf;
    return n;
}

std::string & vformat(std::string & fstr, const char* fmt, va_list arg_list)
{
    char abuf[1024]; abuf[0] = 0;
    int n = vsnprintf(abuf, sizeof(abuf) - 1, fmt, arg_list);
    if (n >= sizeof(abuf)) {
        char * vbuf = NULL;
        n = vpformat(fmt, arg_list, &vbuf, n + 2);
        fstr += vbuf;
        delete [] vbuf;
    } else if (n > 0)
        fstr += abuf;
    return fstr;
}

std::string & format(std::string & fstr, const char* fmt, ...)
{
    va_list arg_list;
    va_start(arg_list, fmt);
    vformat(fstr, fmt, arg_list);
    va_end(arg_list);
    return fstr;
}

/* ************************************************************************** *
 * 
 * ************************************************************************** */
bool readConfig(FILE * fd_cfg, TST_CFG & use_cfg, Logger * use_logger)
{
  char buf[256];
  smsc_log_info(use_logger, "reading config ..\n");

  while (fgets(buf, (int)sizeof(buf), fd_cfg)) {
    CSVList  cmd('=');
    if (cmd.init(buf) < 2) {
      if (cmd.empty() || (cmd[0].c_str()[0] == '#')) {
        continue; //skip blank line or comment
      }
      smsc_log_error(use_logger, "Invalid config parameter: %s", buf);
      return false;
    }

    if (!strcmp("mpUserId", cmd[0].c_str())) {
      int itmp = atoi(cmd[1].c_str());
      if ((itmp <= 0) || (itmp > 19)) {
        smsc_log_error(use_logger, "%s illegal value: %s",
                       cmd[0].c_str(), cmd[1].c_str());
        return false;
      }
      use_cfg.ss7.mpUserId = (uint8_t)itmp;
      smsc_log_info(use_logger, "%s : %u", cmd[0].c_str(),
                    (unsigned)use_cfg.ss7.mpUserId);
      /**/
      use_cfg.mask.bit.ss7_mpUserId = 1;
      continue;
    }

#ifdef EIN_HD
    if (!strcmp("rcpMgrAdr", cmd[0].c_str())) {
      CSVList  adr(',');
      if (adr.init(cmd[1].c_str()) < 1) {
        smsc_log_error(use_logger, "%s illegal value: %s",
                       cmd[0].c_str(), cmd[1].c_str());
        return false;
      }
      for (unsigned i = 0; i < adr.size(); ++i) {
        CSVList hp(':');
        if (hp.init(adr[i].c_str()) < 2) {
          smsc_log_error(use_logger, "%s illegal value: %s",
                         cmd[0].c_str(), adr[i].c_str());
          return false;
        }
      }
      use_cfg.ss7.rcpMgrAdr = adr.print(false);
      smsc_log_info(use_logger, "%s : %s", cmd[0].c_str(),
                    use_cfg.ss7.rcpMgrAdr.c_str());
      /**/
      use_cfg.mask.bit.ss7_rcpMgrAdr = 1;
      continue;
    }
    if (!strcmp("rcpMgrInstId", cmd[0].c_str())) {
      errno = 0;
      int itmp = atoi(cmd[1].c_str());
      if ((!itmp && errno) || (itmp > 255) || (itmp < 0)) {
        smsc_log_error(use_logger, "%s illegal value: %s",
                       cmd[0].c_str(), cmd[1].c_str());
        return false;
      }
      use_cfg.ss7.rcpMgrInstId = (uint8_t)itmp;
      smsc_log_info(use_logger, "%s : %u", cmd[0].c_str(),
                    (unsigned)use_cfg.ss7.rcpMgrInstId);
      /**/
      use_cfg.mask.bit.ss7_rcpMgrInstId = 1;
      continue;
    }
#endif /* EIN_HD */
    if (!strcmp("instIds", cmd[0].c_str())) {
      CSVList  inst(',');
      if (inst.init(cmd[1].c_str()) < 1) {
        smsc_log_error(use_logger, "%s illegal value: %s",
                       cmd[0].c_str(), cmd[1].c_str());
        return false;
      }
#ifdef EIN_HD
      for (unsigned i = 0; i < inst.size(); ++i) {
        int itmp = atoi(inst[i].c_str());
        if (!itmp || (itmp > 255)) {
          smsc_log_error(use_logger, "%s illegal value: %s",
                         cmd[0].c_str(), inst[i].c_str());
          return false;
        }
        use_cfg.ss7.instIds.addInstance((uint8_t)itmp);
      }
      smsc_log_info(use_logger, "%s : %s", cmd[0].c_str(),
                    inst.print(false).c_str());
#else  /* EIN_HD */
      smsc_log_info(use_logger, "%s default value used: 0",
                     cmd[0].c_str());
      use_cfg.ss7.instIds.addInstance(0);
#endif /* EIN_HD */
      /**/
      use_cfg.mask.bit.ss7_instIds = 1;
      continue;
    }
    if (!strcmp("tgtState", cmd[0].c_str())) {
      if (!strcmp("ss7INITED", cmd[1].c_str())) {
        use_cfg.tgtState = TCAPConnector::ss7INITED;
      } else if (!strcmp("ss7REGISTERED", cmd[1].c_str())) {
        use_cfg.tgtState = TCAPConnector::ss7REGISTERED;
      } else if (!strcmp("ss7OPENED", cmd[1].c_str())) {
        use_cfg.tgtState = TCAPConnector::ss7OPENED;
      } else if (!strcmp("ss7CONNECTED", cmd[1].c_str())) {
        use_cfg.tgtState = TCAPConnector::ss7CONNECTED;
      } else {
        smsc_log_error(use_logger, "%s illegal value: %s",
                       cmd[0].c_str(), cmd[1].c_str());
        return false;
      }
      smsc_log_info(use_logger, "%s : %s(%u)", cmd[0].c_str(),
                    cmd[1].c_str(), use_cfg.tgtState);
      /**/
      use_cfg.mask.bit.tst_tgtState = 1;
      continue;
    }
    if (!strcmp("numAttempts", cmd[0].c_str())) {
      int itmp = atoi(cmd[1].c_str());
      if (itmp <= 0) {
        smsc_log_error(use_logger, "%s illegal value: %s",
                       cmd[0].c_str(), cmd[1].c_str());
        return false;
      }
      use_cfg.numAttemts = (unsigned)itmp;
      smsc_log_info(use_logger, "%s : %u", cmd[0].c_str(),
                    use_cfg.numAttemts);
      /**/
      use_cfg.mask.bit.tst_numAttempts = 1;
      continue;
    }
    if (!strcmp("tmoReconn", cmd[0].c_str())) {
      int itmp = atoi(cmd[1].c_str());
      if (itmp <= 0) {
        smsc_log_error(use_logger, "%s illegal value: %s",
                       cmd[0].c_str(), cmd[1].c_str());
        return false;
      }
      use_cfg.tmoReconn = (unsigned)itmp;
      smsc_log_info(use_logger, "%s : %u", cmd[0].c_str(),
                    use_cfg.tmoReconn);
      /**/
      use_cfg.mask.bit.tst_numAttempts = 1;
      continue;
    }
    if (!strcmp("maxFaults", cmd[0].c_str())) {
      int itmp = atoi(cmd[1].c_str());
      if (itmp <= 0) {
        smsc_log_error(use_logger, "%s illegal value: %s",
                       cmd[0].c_str(), cmd[1].c_str());
        return false;
      }
      use_cfg.maxFaults = (unsigned)itmp;
      smsc_log_info(use_logger, "%s : %u", cmd[0].c_str(),
                    use_cfg.maxFaults);
      /**/
      use_cfg.mask.bit.tst_maxFaults = 1;
      continue;
    }
  }
  /**/
  return use_cfg.isInited();
}

/* ************************************************************************** *
 * main():
 * ************************************************************************** */
const char _help[] = "USAGE: einhd_test cfg_file\n";

TCAPConnector _tcCon;

/* function to be called when a connection is broken */
extern "C" unsigned short
  onEINSS7CpConnectBroken(unsigned short from_usrID,
                         unsigned short to_usrID, unsigned char inst_id)
{
  _tcCon.onDisconnect(from_usrID, to_usrID, inst_id);
  return 0;
}


int main(int argc, char ** argv)
{
  if (argc == 1) {
    fprintf(stderr, _help);
    return 0;
  }

  FILE * fdCfg;
  if (!(fdCfg = fopen(argv[1], "r"))) {
    fprintf(stderr, "E: Failed to open file: %s\n", argv[1]);
    return -1;
  }

#ifdef USE_PRIVATE_LOGGER
  Logger::Init();
  Logger * _logger =  Logger::getInstance(LOGGER_CATEGORY);
#else /* USE_PRIVATE_LOGGER */
  Logger * _logger = stdout;
#endif /* USE_PRIVATE_LOGGER */

  TST_CFG _cfg;
  if (!readConfig(fdCfg, _cfg, _logger)) {
    fclose(fdCfg);
    smsc_log_error(_logger, "configuration is incomplete\n");
    return -2;
  }
  smsc_log_info(_logger, "configuration is valid\n");

  /**/
  _tcCon.Init(_cfg.ss7, _logger);
  unsigned errAtt = 0;
  for (unsigned i = 0; i < _cfg.numAttemts; ++i) {
    smsc_log_info(_logger, "run(%u) ..", i);
    if (_tcCon.connectCP(TCAPConnector::ss7CONNECTED) < 0) {
      ++errAtt;
    }
    usleep(_cfg.tmoReconn*1000);
    if (errAtt > _cfg.maxFaults) {
      smsc_log_info(_logger, "run(%u): %u failed reconnect attempts -> full disconnect", i, errAtt);
      _tcCon.disconnectCP(TCAPConnector::ss7None);
      errAtt = 0;
    } else
      _tcCon.disconnectCP(_cfg.tgtState);
    usleep(_cfg.tmoReconn*1000);

    char  nmFile[256];
    strcpy(nmFile, "ehdXXXXXX");
    int fd = mkstemp(nmFile);
    if (fd >= 0) {
      smsc_log_info(_logger, "run(%u): opened file \'%s\', fd : %d", i, nmFile, fd);
      close(fd);
      unlink(nmFile);
    } else {
      smsc_log_error(_logger, "run(%u): unable to open file: %s (%d)",
                     i, strerror(errno), errno);
      break;
    }
  }
  _tcCon.disconnectCP(TCAPConnector::ss7None);
  /**/
  fclose(fdCfg);
  return 0;
}

