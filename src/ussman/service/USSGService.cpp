#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "ussman/service/USSGService.hpp"

#include "ussman/comp/map_uss/MapUSSFactory.hpp"
using smsc::ussman::comp::_ac_map_networkUnstructuredSs_v2;
using smsc::ussman::comp::uss::initMAPUSS2Components;

using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::ReverseMutexGuard;

using smsc::inman::interaction::IProtocolAC;

namespace smsc  {
namespace ussman {

const interaction::INPUSSGateway  USSGService::_iProtoDef; //provided protocol definition


USSGService::~USSGService()
{
  stop(true);

  unsigned connNum = _connPool.usage();
  if (connNum) {
    smsc_log_error(_logger, "%s: %u connects are still alive", _logId, connNum);
  }
}

bool USSGService::init(void)
{
  //MutexGuard grd(_sync);
  if (!_tcDisp.Init(_ss7Cfg, _logger)) {
    smsc_log_fatal(_logger, "%s: TCAPDispatcher initialization failed!", _logId);
    return false;
  }
  if (!_tcDisp.acRegistry()->getFactory(_ac_map_networkUnstructuredSs_v2)
      && !_tcDisp.acRegistry()->regFactory(initMAPUSS2Components)) {
    smsc_log_fatal(_logger, "%s: ROS factory registration failed: %s!", _logId,
                    _ac_map_networkUnstructuredSs_v2.nick());
    return false;
  }
  TcpServerIface::RCode_e rc = _tcpSrv.Init(_tcpCfg);
  if (rc != TcpServerIface::rcOk) {
    smsc_log_fatal(_logger, "%s: TCPServer initialization failed: %u!", _logId, (unsigned)rc);
    return false;
  }
  _tcpSrv.addListener(*this);
  return true;
}


bool USSGService::start()
{
  //MutexGuard grd(_sync);

  if (!_tcDisp.isRunning()) {
    if (!_tcDisp.Start()) {
      smsc_log_fatal(_logger, "%s: TCAPDispatcher start failed!", _logId);
      return false;
    }
  }

  if (!_mgrCfg._wrkCfg._ssnSess) {
    if (!(_mgrCfg._wrkCfg._ssnSess = _tcDisp.openSSN(_mgrCfg._wrkCfg._tcUsr.ownSsn,
                                                     _mgrCfg._wrkCfg._tcUsr.maxDlgId, _logger))) {
      smsc_log_fatal(_logger, "%s: SSN[%u] initialization failure", _logId,
                     (unsigned)_mgrCfg._wrkCfg._tcUsr.ownSsn);
      return false;
    }
  }

  if (_tcpSrv.getState() < TcpServerIface::srvRunning) {
    TcpServerIface::RCode_e rc = _tcpSrv.Start();
    if (rc != TcpServerIface::rcOk) {
      smsc_log_fatal(_logger, "%s: TCPServer start failed: %u!", _logId, (unsigned)rc);
      return false;
    }
  }

  smsc_log_debug(_logger, "%s: Started ..", _logId);
  return true;
}


void USSGService::stop(bool do_wait/* = false*/)
{
  //notify services about stoppping first
  _tcpSrv.Stop(false);
  _tcDisp.Stop(false);

  if (do_wait) { //wait for threads
    _tcpSrv.Stop(true);
    _tcDisp.Stop(true);
    smsc_log_debug(_logger, "%s: Stopped.", _logId);
  } else {
    smsc_log_debug(_logger, "%s: Stopping ..", _logId);
  }
}


//Creates a connect manager serving given connect.
//Returns true on success, false -  otherwise.
bool USSGService::setConnListener(const ConnectGuard & use_conn) /*throw()*/
{
//  MutexGuard  grd(_sync);
  USSConnManager * pMgr = _sessReg.find(use_conn->getUId());
  if (pMgr) {
    smsc_log_warn(_logger, "%s: %s is already opened on Connect[%u]",
                  _logId,  pMgr->logId(), (unsigned)use_conn->getUId());
    _sync.notify();
    return false;
  }
  //create new connect manager
  pMgr = new USSConnManager(_mgrCfg, _iProtoDef, ++_lastSessId, _logger);
  _sessReg.insert(use_conn->getUId(), pMgr);
  pMgr->bind(&use_conn);
  pMgr->start(); //also switches Connect to asynchronous mode
  smsc_log_info(_logger, "%s: %s is started on Connect[%u]",
                _logId, pMgr->logId(), (unsigned)use_conn->getUId());
  _sync.notify();
  return true;
}

//Handles USSConnManager destruction upon disconnection.
void USSGService::onDisconnect(const ConnectGuard & use_conn) /*throw()*/
{
  std::auto_ptr<USSConnManager> pMgr(_sessReg.extract(use_conn->getUId()));
  if (pMgr.get()) {
    ReverseMutexGuard  grd(_sync);
    pMgr->bind(NULL);
    pMgr->stop(false);
    smsc_log_info(_logger, "%s: closing %s on Connect[%u]",
                   _logId,  pMgr->logId(), (unsigned)use_conn->getUId());
    pMgr.reset(); //USSConnManager is destroyed at this point
  }
  _sync.notify();
}

// --------------------------------------------------------------------------
// -- TcpServerListenerIface interface mthods
// --------------------------------------------------------------------------
SocketListenerIface *
  USSGService::onConnectOpening(TcpServerIface & p_srv, ConnectUId conn_id,
                                std::auto_ptr<Socket> & use_sock)
{
  MutexGuard  grd(_sync);
  //cleanUp died connects first
  while (!_corpses.empty()) {
  #ifdef __GRD_POOL_DEBUG__
    smsc_log_debug(_logger, "%s: onConnectOpening() corpses cleanUp ..", _logId);
    _corpses.front().logThis(_logger, _logId);
  #endif /* __GRD_POOL_DEBUG__ */
    _corpses.pop_front();
  }

  ConnectGuard connGrd = _connPool.allcObj();
  #ifdef __GRD_POOL_DEBUG__
  smsc_log_debug(_logger, "%s: onConnectOpening(%u)", _logId, conn_id);
  connGrd.logThis(_logger, _logId);
  #endif /* __GRD_POOL_DEBUG__ */
  //set consequtive processing of incoming packets
  connGrd->init(conn_id, _pckPool, 1, _logger);
  connGrd->bind(use_sock);
  //listen for 1st incoming packet in order to detect required protocol
  connGrd->addListener(*this);
  if (connGrd->start()) {
    _connMap.insert(ConnectsMap::value_type(connGrd->getUId(), connGrd));
    smsc_log_info(_logger, "%s: activated %s", _logId, connGrd->logId());
    return connGrd.get();
  }
  smsc_log_fatal(_logger, "%s: failed to activate %s", _logId, connGrd->logId());
  return NULL;
}

//Notifies that connection is to be closed on given soket, no more events will be reported.
void USSGService::onConnectClosing(TcpServerIface & p_srv, ConnectUId conn_id)
{
  MutexGuard grd(_sync);
  //cleanUp died connects first
  while (!_corpses.empty()) {
  #ifdef __GRD_POOL_DEBUG__
    smsc_log_debug(_logger, "%s: onConnectClosing() corpses cleanUp ..", _logId);
    _corpses.front().logThis(_logger, _logId);
  #endif /* __GRD_POOL_DEBUG__ */
    _corpses.pop_front();
  }

  ConnectsMap::iterator it = _connMap.find(conn_id);
  if (it == _connMap.end()) //Connect already unregistered by onConnectError()
    return;

  #ifdef __GRD_POOL_DEBUG__
  smsc_log_debug(_logger, "%s: onConnectClosing(%u)", _logId, conn_id);
  it->second.logThis(_logger, _logId);
  #endif /* __GRD_POOL_DEBUG__ */
  ConnectGuard rConn = it->second;
  rConn->removeListener(*this);
  //Note: Connect object MUST not be destroyed inside this thread,
  //so postpone its destruction.
  _corpses.push_back(it->second);
  _connMap.erase(it);
  smsc_log_info(_logger, "%s: unregistering Connect[%u]", _logId, conn_id);
  onDisconnect(rConn);
}

//notifies that TcpServer is shutdowned, no more events on any connect will be reported.
void USSGService::onServerShutdown(TcpServerIface & p_srv, TcpServerIface::RCode_e down_reason)
{
  _tcpSrv.removeListener(*this);
  MutexGuard  grd(_sync);
  //cleanUp died connects first
  while (!_corpses.empty())
    _corpses.pop_front();

  smsc_log_debug(_logger, "%s: TCP server shutdowned, reason %d", _logId, down_reason);
}

// --------------------------------------------------------------------------
// -- PacketListenerIface interface mthods
// --------------------------------------------------------------------------
//Returns true if listener has utilized packet so no more listeners
//should be notified, false - otherwise (in that case packet will be
//reported to other listeners).
bool USSGService::onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
  /*throw(std::exception) */
{
  MutexGuard  grd(_sync);
  ConnectsMap::iterator it = _connMap.find(conn_id);
  if (it == _connMap.end()) { //internal inconsistency
    smsc_log_warn(_logger, "%s: packet is received on unregistered Connect[%u]",
                  _logId, conn_id);
    return true;
  }
  ConnectGuard  rConn = it->second;
  rConn->removeListener(*this);

  IProtocolAC::PduId pduId = _iProtoDef.isKnownPacket(recv_pck);
  if (!pduId) {
    smsc_log_error(_logger, "%s: unknown packet is received on Connect[%u]",
                  _logId, conn_id);
    _tcpSrv.rlseConnectionNotify(conn_id, true);
    return true;
  }
  if (pduId != interaction::USSRequestMessage::_cmdTAG) {
    smsc_log_error(_logger, "%s: illegal Cmd[%u] received", _logId, (unsigned)pduId);
    _tcpSrv.rlseConnectionNotify(conn_id, true);
    return true;
  }

  bool isAccepted = setConnListener(rConn);
  if (!isAccepted) {
    smsc_log_warn(_logger, "%s: denied Connect[%u]", _logId, conn_id);
    _tcpSrv.rlseConnectionNotify(conn_id, true);
  }
  return !isAccepted; //if ok, report packet to newly created connect listener
}

//Returns true if listener has processed connect exception so no more
//listeners should be notified, false - otherwise (in that case exception
//will be reported to other listeners).
bool USSGService::onConnectError(unsigned conn_id,
                                  PckAccumulatorIface::Status_e err_status,
                                  const CustomException * p_exc/* = NULL*/)
  /*throw(std::exception) */
{
  if ((err_status != PckAccumulatorIface::accEOF) || p_exc) {
    smsc_log_error(_logger, "%s: Connect[%u] error status(%u): %s", _logId, conn_id,
                   (unsigned)err_status, p_exc ? p_exc->what() : "");
  }
  MutexGuard  grd(_sync);
  ConnectsMap::iterator it = _connMap.find(conn_id);
  if (it == _connMap.end()) //Connect already unregistered by onConnectClosing()
    return true;

  it->second->removeListener(*this);
  //Note: Connect object MUST not be destroyed inside this method,
  //so postpone its destruction.
  _corpses.push_back(it->second);
  _connMap.erase(it);
  smsc_log_info(_logger, "%s: unregistering Connect[%u]", _logId, conn_id);
  return true;
}

} //ussman
} //smsc

