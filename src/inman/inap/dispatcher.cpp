#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/inap/dispatcher.hpp"
#include "inman/inap/ACRegistry.hpp"
#include "inman/inap/TCAPErrors.hpp"
#include "inman/inap/TCCallbacks.hpp"
using smsc::core::synchronization::ReverseMutexGuard;

namespace smsc  {
namespace inman {
namespace inap  {

#define MAX_BIND_TIMEOUT  6000  //maximum timeout on SSN bind request, units: millisecs
#define MAX_CONN_TIMEOUT  6000  //maximum timeout on Unit reconnection 
#define RECV_TIMEOUT      300   //message receiving timeout, units: millisecs

#ifdef EIN_HD
# define SS7_RESET_STAGE ss7None        //total reset (include CP static data)
#else
# define SS7_RESET_STAGE ss7REGISTERED  //msg ports/connections reset
#endif

/* ************************************************************************** *
 * class TCAPDispatcher::MessageListener implementation:
 * ************************************************************************** */
bool TCAPDispatcher::MessageListener::isRunning(void) const
{
  MutexGuard tmp(_sync);
  return _running;
}

bool TCAPDispatcher::MessageListener::Start(void)
{
  {
    MutexGuard grd(_sync);
    if (_running)
      return true;
  }
  Thread::Start();
  {
    MutexGuard grd(_sync);
    if (!_running)
      _sync.wait(RECV_TIMEOUT);
    return _running;
  }
}

void TCAPDispatcher::MessageListener::Stop(bool do_wait/* = false*/)
{
  {
    MutexGuard grd(_sync);
    if (_running) {
      _running = false;
      _sync.notify();
    }
  }
  if (do_wait)
    Thread::WaitFor();
}

int TCAPDispatcher::MessageListener::Execute(void)
{
  unsigned short result = 0;

  _sync.Lock(); //waits here for Start()
  _running = true;
  _sync.notify();

  while (_running) {
    _sync.Unlock();
    if (!_dsp.msgListeningOn()) {
      _sync.Lock();
      _sync.wait(RECV_TIMEOUT);
      continue;
    }
    result = _dsp.dispatchMsg();
    _sync.Lock();
  }
  if (MSG_TIMEOUT == result)
    result = 0;
  _sync.Unlock();
  return result;
}

/* ************************************************************************** *
 * class TCAPDispatcher implementation:
 * ************************************************************************** */
TCAPDispatcher::TCAPDispatcher()
    : _ss7State(ss7None), _dspState(dspStopped), _connCounter(0)
    , _maxUConnAtt(0), _unitCfg(NULL)
#ifdef EIN_HD
    , _rcpMgrAdr(NULL)
#endif /* EIN_HD */
    , _logId("TCAPDsp"), logger(Logger::getInstance("smsc.inman.inap")), _msgAcq(*this)
{
    ApplicationContextRegistry::get();
    TCCbkLink::get(); //Link TC API Callbacks
}

TCAPDispatcher::~TCAPDispatcher()
{
    Stop(true);
    MutexGuard grd(_sync);
    disconnectCP(ss7None);
#ifdef EIN_HD
    delete [] _rcpMgrAdr;
#endif /* EIN_HD */
    TCCbkLink::get().DeInit();
    smsc_log_info(logger, "%s: SS7 stack disconnected", _logId);
}

ApplicationContextRegistryITF * TCAPDispatcher::acRegistry(void) const
{
    return &ApplicationContextRegistry::get();
}

    
//Initializes TCAPDispatcher and SS7 communication facility
//Returns true on success
bool TCAPDispatcher::Init(const TCDsp_CFG & use_cfg, Logger * use_log/* = NULL*/)
{
    if (TCCbkLink::get().tcapDisp()) //TCAPDispatcher is already inited!
      return false;

    MutexGuard grd(_sync);
    _cfg = use_cfg;
    _maxUConnAtt = ((MAX_CONN_TIMEOUT + _cfg.tmoReconn/2)/_cfg.tmoReconn);
    _cfg.mpUserId += USER01_ID - 1; //adjust USER_ID to PortSS7 units id
#ifdef EIN_HD
    _rcpMgrAdr = new char[_cfg.rcpMgrAdr.length() + 2];
    strcpy(_rcpMgrAdr, _cfg.rcpMgrAdr.c_str());
#else /* EIN_HD */
    //set the single TCAP BE instance with id = 0
    SS7Unit_CFG tcUnit(TCAP_ID);
    tcUnit.instIds.insert(SS7UnitInstance(0));
    _cfg.ss7Units[TCAP_ID] = tcUnit;
#endif /* EIN_HD*/

    //check for TCAP BE instances ids
    SS7UnitsCFG::iterator it = _cfg.ss7Units.find(TCAP_ID);
    if ((it == _cfg.ss7Units.end()) || it->second.instIds.empty())
        return false;
    _unitCfg = &(it->second);

    if (use_log)
      logger = use_log;

    if (connectCP(TCAPDispatcherITF::ss7REGISTERED) < 0)
        return false;
    TCCbkLink::get().Init(this, logger);
    return true;
}

//Returns true on successfull connection to TCAP unit of SS7 stack,
//starts TCAP messages(indications) listener
bool TCAPDispatcher::Start(void)
{
  {
    MutexGuard grd(_sync);
    if (_dspState == dspRunning)
      return true;
    if (connectCP(ss7OPENED) < 0)
      return false;
    smsc_log_debug(logger, "%s: starting MsgListener thread ..", _logId);
    if (!_msgAcq.Start())
      return false;
    smsc_log_debug(logger, "%s: starting Autoconnection thread..", _logId);
    Thread::Start();
  }
  {
    MutexGuard grd(_sync);
    if (_dspState == dspRunning)
      return true;
    _sync.wait(RECV_TIMEOUT);
    if (_dspState != dspRunning) {
      smsc_log_fatal(logger, "%s: unable to start", _logId);
      return false;
    }
  }
  return true;
}

//Stopps dispatcher in two phases. 2nd is performed only if do_wait is set
//1) Stops Units autoconnection thread, switches SSNSessions to stopping state 
//2) Destroys SSNSessions, finishes autoconnection and listener threads, 
//   switches to ss7INITED state.
TCAPDispatcherITF::DSPState_e TCAPDispatcher::Stop(bool do_wait/* = false*/)
{
  {
    MutexGuard grd(_sync);
    if (_dspState == dspStopped)
      return _dspState;
    if (_dspState == dspRunning) {
      //stop reconnection thread, keep listener thread active for a while
      _dspState = dspStopping;
      smsc_log_debug(logger, "%s: stopping ..", _logId);
      //switch all active sessions to stopping state
      if (!_sessions.empty()) {
        for (SSNmap_T::size_type i = 0; i < _sessions.size(); ++i)
          _sessions[i]->Stop();
      }
      _sync.notify(); //notify reconnection thread about stopping
    }
  }
  if (do_wait) {
    { //delete all sessions
      MutexGuard grd(_sync);
      unbindSSNs();
      while (!_sessions.empty()) {
        SSNmap_T::size_type idx = _sessions.size() - 1;
        SSNSession * pSession = _sessions[idx];
        smsc_log_debug(logger, "%s: destroying SSN[%u], sessIdx = %u", _logId,
                       (unsigned)(pSession->getSSN()), (unsigned)idx);
        _sessions.erase(idx);
        {
          ReverseMutexGuard rGrd(_sync);
          delete pSession;
        }
      }
    }
    //wait for Autoconnection and Listener threads
    Thread::WaitFor();
    _msgAcq.Stop(true);
    smsc_log_debug(logger, "%s: MsgListener thread finished", _logId);
    {
      MutexGuard grd(_sync);
      _dspState = dspStopped;
      disconnectCP(ss7INITED);
    }
  }
  return _dspState;
}

// -------------------------------------------------------------------------
// TCAP messages(indications) listener methods
// -------------------------------------------------------------------------
//Returns true if message listening mode is ON
bool TCAPDispatcher::msgListeningOn(void) const
{
  MutexGuard grd(_sync);
  return (_dspState != dspStopped) && (_ss7State == ss7CONNECTED);
}

//This method is always called from MsgListener thread
unsigned short TCAPDispatcher::dispatchMsg(void)
{
  MSG_T msg;
  memset(&msg, 0, sizeof(MSG_T));
  msg.receiver = _cfg.mpUserId;

#ifdef EIN_HD
  USHORT_T result = EINSS7CpMsgRecv_r(&msg, RECV_TIMEOUT);
#else  /* EIN_HD */
  USHORT_T result = 0;
  {
    MutexGuard  tmp(_msgRecvLock);
    result = EINSS7CpMsgRecv_r(&msg, RECV_TIMEOUT);
  }
#endif /* EIN_HD */
  if (MSG_TIMEOUT != result) {
    if (!result) {
      if (msg.sender == TCAP_ID)
        EINSS7_I97THandleInd(&msg); //calls TCCallbacks.cpp::*()
      else
        smsc_log_warn(logger, "%s: ignoring msg[%u] received from unit[%u], ",
                        _logId, (unsigned)msg.primitive, (unsigned)msg.sender);
    }
    EINSS7CpReleaseMsgBuffer(&msg);
    if ((MSG_BROKEN_CONNECTION == result) || (MSG_NOT_CONNECTED == result))
      result = MSG_TIMEOUT;
    else if (result) {
      smsc_log_error(logger, "%s: MsgRecv() failed with code %u (%s)", _logId,
                      result, rc2Txt_SS7_CP(result));
    }
  }
  return result;
}

int TCAPDispatcher::Reconnect(void)
{
//  unsigned connCounter = 0;
  unsigned maxBindAttempts = ((MAX_BIND_TIMEOUT + _cfg.tmoReconn/2)/_cfg.tmoReconn);
  unsigned bindCounter = 0;

  _sync.Lock(); //waits here for Start()
  _dspState = dspRunning;
  _sync.notify();
  while (_dspState == dspRunning) {
//smsc_log_debug(logger, "%s: Reconnect(): state %u, connCounter: %u", _logId, _ss7State, _connCounter);
    if ((_connCounter >= _maxUConnAtt) || (bindCounter >= maxBindAttempts)) {
      smsc_log_error(logger, "%s: lingering connection troubles, reconnecting ..", _logId);
      disconnectCP(SS7_RESET_STAGE);
      _connCounter = bindCounter = 0;
    } else if (_ss7State == ss7CONNECTED) {
      //check for disconnected units
      if (disconnectedUnits()) {
        if (!connectUnits()) {
          _sync.wait(_cfg.tmoReconn);
          continue;
        }
        //give the SCCP time to refresh SubSystems states prior to rebinding them.
        _sync.wait((_cfg.tmoReconn*2)/3);
      }
      if (unitsNeedBinding())
        bindSSNs();
      if (!_sessions.empty() && (unbindedSSNs() == _sessions.size()))
        ++bindCounter;
    } else if (connectCP(ss7CONNECTED) < 0) { //also binds SSNs
      ++_connCounter;
    } else
      _msgAcq.Notify();
    /**/
    _sync.wait(_cfg.tmoReconn);
  }
  _sync.Unlock();
  return 0;
}

void TCAPDispatcher::onDisconnect(unsigned char inst_id)
{
  MutexGuard grd(_sync);
  SS7UnitInstance * pInst = _unitCfg->instIds.getInstance(inst_id);
  if (pInst && (pInst->connStatus == SS7UnitInstance::uconnOk)) {
    pInst->connStatus = SS7UnitInstance::uconnError;
    logger->log((_dspState == dspRunning) ? Logger::LEVEL_ERROR : Logger::LEVEL_DEBUG,
                "%s: disconnected userId=%u -> TCAP[instId = %u]",
                _logId, _cfg.mpUserId, inst_id);

    for (SSNmap_T::size_type i = 0; i < _sessions.size(); ++i) {
      SSNSession * pSess = _sessions[i];
      pSess->ResetUnit(inst_id, UNITStatus::unitError);
      if (pSess->bindStatus() < SSNBinding::ssnPartiallyBound) {
        smsc_log_error(logger, "%s: SSN[%u] has been disbound!", _logId, (unsigned)pSess->getSSN());
      }
    }
    //check for last instance disconnection
    bool allBroken = false;
    if (disconnectedUnits(&allBroken) && allBroken) {
      if (_ss7State == ss7CONNECTED)
        _ss7State = ss7OPENED; //forbid message listening, and avoid Unbind/RelInst calls
      _connCounter = _maxUConnAtt;  //force total reconnection in Reconnect()
    }
    _sync.notify(); //awake reconnector thread
  } else {
    smsc_log_debug(logger, "%s: connection broken userId=%u -> TCAP[instId = %u]",
                  _logId, _cfg.mpUserId, inst_id);
  }
}

void TCAPDispatcher::onDisconnect(unsigned short from_usrID,
                         unsigned short to_usrID, unsigned char inst_id)
{
  if ((from_usrID != _cfg.mpUserId) || (to_usrID != TCAP_ID)) {
    smsc_log_warn(logger, "%s: connection broken userId=%u -> userId=%u[instId = %u]",
                   _logId, from_usrID, to_usrID, inst_id);
    return;
  }
  onDisconnect(inst_id);
}

// Listener thread entry point
int TCAPDispatcher::Execute(void)
{
    int result = 0;
    try {
        smsc_log_info(logger, "%s: Autoconnection thread started", _logId);
        result = Reconnect();
    } catch (const std::exception& exc) {
        smsc_log_fatal(logger, "%s: Autoconnection thread got an exception: %s", _logId, exc.what() );
        result = -2;
    }
    smsc_log_debug(logger, "%s: Autoconnection thread finished, reason %d", _logId, result);
    return result;
}

/* -------------------------------------------------------------------------- *
 * PRIVATE:
 * -------------------------------------------------------------------------- */
//Checks for unconnected TCAP BE instances and returns its total number
unsigned TCAPDispatcher::disconnectedUnits(bool * is_all/* = NULL*/) const
{
    unsigned rval = 0, i = 0;
    for (SS7UnitInstsMap::size_type
          it = 0; it < _unitCfg->instIds.size(); ++it, ++i) {
      if (_unitCfg->instIds[it].connStatus != SS7UnitInstance::uconnOk)
        ++rval;
    }
    if (is_all)
      *is_all = (rval == i);
//smsc_log_debug(logger, "%s: disconnectedUnits(): %u {%s}", _logId, rval, rval == i ? "ALL" : " ");
    return rval;
}

//Connects currently disconnected TCAP BE instances.
//Returns number of new instances succesfully connected
//NOTE: _sync MUST be locked upon entry!
unsigned TCAPDispatcher::connectUnits(void)
{
  unsigned cnt = 0;
  for (SS7UnitInstsMap::size_type it = 0; it < _unitCfg->instIds.size(); ++it) {
//    smsc_log_debug(logger, "%s: connectUnits() unit[%u] status: %u", _logId,
//                   it->second.instId, it->second.connStatus);
    SS7UnitInstance & unitInst = _unitCfg->instIds[it];
    if (unitInst.connStatus >= SS7UnitInstance::uconnAwaited)
      continue;
    unitInst.connStatus = SS7UnitInstance::uconnAwaited;
    USHORT_T result = 0;
    {
      ReverseMutexGuard rgrd(_sync);
#ifdef EIN_HD
      result = EINSS7CpMsgConnNotify(_cfg.mpUserId, TCAP_ID,
                                     unitInst.instId, onEINSS7CpConnectBroken);
#else  /* EIN_HD */
      {
        MutexGuard  tmp(_msgRecvLock);
        result = EINSS7CpMsgConnNotify(_cfg.mpUserId, TCAP_ID,
                                       unitInst.instId, onEINSS7CpConnectBroken);
      }
#endif /* EIN_HD */
    }
    if (result != 0) {
      smsc_log_error(logger, "%s: MsgConn(TCAP instId = %u) failed: %s (code %u)",
                  _logId, (unsigned)unitInst.instId, rc2Txt_SS7_CP(result), result);
      _unitCfg->instIds[it].connStatus = SS7UnitInstance::uconnError;
    } else {
      smsc_log_info(logger, "%s: MsgConn(TCAP instId = %u) Ok!",
                  _logId, (unsigned)unitInst.instId);
      _unitCfg->instIds[it].connStatus = SS7UnitInstance::uconnOk;
      ++cnt;
    }
  }
//  smsc_log_debug(logger, "%s: connectUnits() {%u}", _logId, cnt);
  return cnt;
}

//Disconnects all TCAP BE instances.
//NOTE: _sync MUST be locked upon entry! 
void TCAPDispatcher::disconnectUnits(void)
{
  for (SS7UnitInstsMap::size_type it = 0; it < _unitCfg->instIds.size(); ++it) {
    SS7UnitInstance & unitInst = _unitCfg->instIds[it];
    if (unitInst.connStatus == SS7UnitInstance::uconnOk) {
      USHORT_T result = 0;
      {
        ReverseMutexGuard rgrd(_sync);
  #ifdef EIN_HD
        result = EINSS7CpMsgRelInst(_cfg.mpUserId, TCAP_ID, unitInst.instId);
  #else  /* EIN_HD */

        { //NOTE: wait for CpMsgRecv completion, in order to avoid EIN SS7 internal deadlock
          MutexGuard  grd(_msgRecvLock);
          result = EINSS7CpMsgRelInst(_cfg.mpUserId, TCAP_ID, unitInst.instId);
        }
  #endif /* EIN_HD */
      }
      if (result) {
          smsc_log_error(logger, "%s: MsgRel(TCAP instId = %u) failed: %s (code %u)",
                      _logId, (unsigned)unitInst.instId, rc2Txt_SS7_CP(result), result);
      }
    }
    unitInst.connStatus = SS7UnitInstance::uconnIdle;
  }
  return;
}

//Check all connected unit instances for binding status
//Returns true if at least one unit instance needs binding
//NOTE: _sync MUST be locked upon entry! 
bool TCAPDispatcher::unitsNeedBinding(void) const
{
  for (SS7UnitInstsMap::size_type it = 0; it < _unitCfg->instIds.size(); ++it) {
    const SS7UnitInstance & unitInst = _unitCfg->instIds[it];
    if (unitInst.connStatus == SS7UnitInstance::uconnOk) {
      for (SSNmap_T::size_type sit = 0; sit < _sessions.size(); ++sit) {
        const UNITBinding * unb = _sessions[sit]->findUnit(unitInst.instId);
        if (unb && (unb->getUnit()._bindStatus < UNITStatus::unitAwaited))
          return true;
      }
    }
  }
  return false;
}

//Checks all SubSystems for bind status.
//Returns number of unbided SSNs. 
//NOTE: _sync MUST be locked upon entry!
unsigned TCAPDispatcher::unbindedSSNs(void) const
{
    unsigned rval = 0;
    for (SSNmap_T::size_type it = 0; it < _sessions.size(); ++it) {
      if (_sessions[it]->bindStatus() < SSNBinding::ssnPartiallyBound)
        ++rval;
    }
    return rval;
}

//Binds SubSystem to each unit instance.
//Returns true if at least one BindReq() is successfull
//NOTE: actual BindReq() result is returned to confirmSSN()
//NOTE: _sync MUST be locked upon entry! 
bool TCAPDispatcher::bindSSN(SSNSession * p_session) const
{
    if (_ss7State != ss7CONNECTED)
      return false;

    bool rval = false;
    UnitsStatus tcUnits;
    p_session->getUnitsStatus(tcUnits);

    for (UnitsStatus::const_iterator cit = tcUnits.begin();
                                      cit != tcUnits.end() ; ++cit) {
      const UNITStatus & unit = *cit;
//smsc_log_debug(logger, "%s: bindSSN(SSN=%u, instId=%u, status=%u)", _logId,
//               p_session->getSSN(), (unsigned)unit.instId, unit.bindStatus);
      if (unit._bindStatus >= UNITStatus::unitAwaited)
        continue;
      if (!_unitCfg->instIds.isStatus(unit._instId, SS7UnitInstance::uconnOk))
        continue;
      if (unit._bindStatus == UNITStatus::unitError)
        p_session->ResetUnit(unit._instId);

      USHORT_T result = 0;
      {
        ReverseMutexGuard rgrd(_sync);
        result = EINSS7_I97TBindReq(p_session->getSSN(), _cfg.mpUserId,
                                              unit._instId, EINSS7_I97TCAP_WHITE_USER
  #if EIN_HD >= 101
                                              , 0
  #endif /* EIN_HD >= 101 */
                                              );
      }
      if (result) {
        smsc_log_error(logger, "%s: TBindReq(SSN=%u, userId=%u,"
                               " instId=%u) failed with code %u (%s)", _logId,
                        p_session->getSSN(), _cfg.mpUserId, (unsigned)unit._instId,
                        result, rc2Txt_TC_APIError(result));
         //no confirmation will come from EIN SS7, so rise signal!
        p_session->SignalUnit(unit._instId, UNITStatus::unitError);
      } else {
        smsc_log_info(logger, "%s: TBindReq(SSN=%u, userId=%u, instId=%u) Ok!", _logId,
                       p_session->getSSN(), _cfg.mpUserId, (unsigned)unit._instId);
        p_session->setUnitStatus(unit._instId, UNITStatus::unitAwaited);
        rval = true;  //await confirmation
      }
    }
    return rval;
}

//Binds all SubSystems to each unit instance.
//Returns true if at least one BindReq() succeeded
//NOTE: _sync MUST be locked upon entry!
bool TCAPDispatcher::bindSSNs(void) const
{
    if (_ss7State != ss7CONNECTED)
      return false;

    bool failed = false;
    for (SSNmap_T::size_type it = 0; it < _sessions.size(); ++it)
      failed |= !bindSSN(_sessions[it]);
    return !failed;
}

//Unbinds SubSystem from each unit instance.
//NOTE: _sync MUST be locked upon entry!
void TCAPDispatcher::unbindSSN(SSNSession * p_session) const
{
    UnitsStatus tcUnits;
    p_session->getUnitsStatus(tcUnits);

    for (UnitsStatus::const_iterator cit = tcUnits.begin();
                                        cit != tcUnits.end(); ++cit) {
      const UNITStatus & unit = *cit;
      if (unit._bindStatus == UNITStatus::unitBound) {
        USHORT_T result = 0;
        {
          ReverseMutexGuard rgrd(_sync);
          result = EINSS7_I97TUnBindReq(p_session->getSSN(), _cfg.mpUserId, unit._instId);
        }
        if (!result)
          smsc_log_debug(logger, "%s: TUnBindReq(SSN=%u, userId=%u, instId=%u)", _logId,
                         p_session->getSSN(), _cfg.mpUserId, (unsigned)unit._instId);
        else
          smsc_log_error(logger, "%s: TUnBindReq(SSN=%u, userId=%u, instId=%u)"
                                       " failed with code %u (%s)", _logId,
                         p_session->getSSN(), _cfg.mpUserId, (unsigned)unit._instId,
                         result, rc2Txt_TC_APIError(result));
      }
      p_session->ResetUnit(unit._instId);
    }
    return;
}
//Unbinds all SubSystems from each unit instance.
//NOTE: _sync MUST be locked upon entry!
void TCAPDispatcher::unbindSSNs(void) const
{
  for (SSNmap_T::size_type it = 0; it < _sessions.size(); ++it)
    unbindSSN(_sessions[it]);
}

//Returns:  (-1) - failed to connect, 0 - already connected, 1 - successfully connected
//NOTE: _sync MUST be locked upon entry!
int TCAPDispatcher::connectCP(SS7State_e upTo/* = ss7CONNECTED*/)
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
        _msgAcq.Notify(); //awake MsgListener
        smsc_log_info(logger, "%s: state CONNECTED (userId = %u)", _logId, _cfg.mpUserId);
        bindSSNs();
      } break;

      default:;
      } /* eosw */
    }
    return rval;
}
//NOTE: _sync MUST be locked upon entry!
void TCAPDispatcher::disconnectCP(SS7State_e downTo/* = ss7None*/)
{
    if (_ss7State > downTo) {
        smsc_log_info(logger, "%s: disconnecting SS7 stack: %u -> %u ..", _logId, _ss7State, downTo);
    }
    while (_ss7State > downTo) {
      switch (_ss7State) {
      case ss7CONNECTED: { //Releasing of connections to other users.
        unbindSSNs();
        disconnectUnits();
        smsc_log_info(logger, "%s: state OPENED", _logId);
        _ss7State = ss7OPENED;
      } break;

      case ss7OPENED: { //Closing of the input queue.
        USHORT_T result;
        if ((result = MsgClose(_cfg.mpUserId)) != 0) {
            smsc_log_error(logger, "%s: MsgClose(userId = %u) failed: %s (code %u)",
                           _logId, (unsigned)_cfg.mpUserId, 
                           rc2Txt_SS7_CP(result), result);
        }
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

/* ************************************************************************* *
 * TCAP sessions factory methods
 * ************************************************************************* */
//Binds SSN to all known TCAP BE instances and initializes SSNSession (TCAP dialogs factory)
SSNSession* TCAPDispatcher::openSSN(UCHAR_T ssn_id, USHORT_T max_dlg_id/* = 2000 */,
                                    Logger * uselog/* = NULL*/)
{
  SSNSession* pSession = NULL;
  {
    MutexGuard tmp(_sync);
    if (_dspState != dspRunning)
      return NULL;

    if (!(pSession = _sessions.find(ssn_id))) {
      pSession = new SSNSession(this->acRegistry() , ssn_id, _cfg.mpUserId,
                                _cfg.ss7Units[TCAP_ID].instIds,
                                max_dlg_id, uselog ? uselog : logger);
      if (_sessions.insert(pSession) == _sessions.npos()) {
        smsc_log_fatal(logger, "%s: failed to open SSN[%u], too much SSNs opened: %u",
                       _logId, ssn_id, (unsigned)_sessions.size());
        delete pSession;
        return NULL;
      }
    } else {
      pSession->incMaxDlgs(max_dlg_id);
      smsc_log_debug(logger, "%s: SSN[%u] already inited, state: %u", _logId,
                     ssn_id, pSession->bindStatus());
    }
  }
  if (pSession->bindStatus() < SSNBinding::ssnPartiallyBound) {
    _sync.notify(); //awake autoconnection thread
    pSession->Wait(MAX_BIND_TIMEOUT);
  }
  return pSession;
}

SSNSession* TCAPDispatcher::findSession(UCHAR_T ssn) const
{
  MutexGuard tmp(_sync);
  return _sessions.find(ssn);
}

void TCAPDispatcher::confirmSSN(uint8_t ssn, uint8_t tc_inst_id, uint8_t bindResult)
{
  SSNSession * pSession = findSession(ssn);
  if (!pSession) {
    smsc_log_warn(logger, "%s: TBindConf(SSN=%u, userId=%u, instId=%u) : invalid/inactive SSN",
                 _logId, (unsigned)ssn, _cfg.mpUserId, (unsigned)tc_inst_id);
    return;
  }
  const UNITBinding * unb = pSession->findUnit(tc_inst_id);
  if (!unb) {
    smsc_log_warn(logger, "%s: TBindConf(SSN=%u, userId=%u, instId=%u) : invalid/inactive instance",
                  _logId, (unsigned)ssn, _cfg.mpUserId, (unsigned)tc_inst_id);
    return;
  }

  UNITStatus::BindStatus_e rval = UNITStatus::unitError;

  if ((bindResult == TC_BindResult::bindOk)
      || (bindResult == TC_BindResult::ssnInUse)) {
    rval = UNITStatus::unitBound;
    _msgAcq.Notify();
    smsc_log_info(logger, "%s: TBindConf(SSN=%u, userId=%u, instId=%u) Ok!", _logId,
                   (unsigned)ssn, _cfg.mpUserId, (unsigned)tc_inst_id);
  } else {
    if (bindResult == TC_BindResult::tcUserIdErr) {
      //NOTE: the EINHD SS7 r4 has a bug that results in a duplicate
      //      BindConf_Ind{tcUserIdErr} recieved
      if (unb->isSignaled(UNITStatus::unitBound)) {
        smsc_log_warn(logger, "%s: TBindConf(SSN=%u, userId=%u, instId=%u) ack: '%s' (code 0x%X)",
                       _logId, (unsigned)ssn, _cfg.mpUserId, (unsigned)tc_inst_id,
                       rc2Txt_TC_BindResult(bindResult), bindResult);
        return;
      }
    }
    smsc_log_error(logger, "%s: TBindConf(SSN=%u, userId=%u, instId=%u) failed: '%s' (code 0x%X)",
                   _logId, (unsigned)ssn, _cfg.mpUserId, (unsigned)tc_inst_id,
                   rc2Txt_TC_BindResult(bindResult), bindResult);
  }
  pSession->SignalUnit(tc_inst_id, rval);
}

} // namespace inap
} // namespace inmgr
} // namespace smsc

