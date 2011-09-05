#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/tcpsrv/ICSTcpSrv.hpp"
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::ReverseMutexGuard;

namespace smsc  {
namespace inman {
namespace tcpsrv {
/* ************************************************************************** *
 * class ICSTcpServer implementation:
 * ************************************************************************** */
ICSTcpServer::~ICSTcpServer()
{
  ICSStop(true);
  {
    MutexGuard grd(_sync);
    //cleanUp died connects
    _corpses.clear();
  }
}

// ---------------------------------------------
// -- ICServiceAC interface methods
// --------------------------------------------- 
//NOTE: all methods are called with _sync locked!
ICServiceAC::RCode ICSTcpServer::_icsInit(void)
{
  //initialize Service socket, set TCP server listener 
  if (_tcpSrv.Init(*_cfg.get()) == TcpServer::rcOk) {
    _tcpSrv.addListener(*this);
    return ICServiceAC::icsRcOk;
  }
  return ICServiceAC::icsRcError;
}

ICServiceAC::RCode ICSTcpServer::_icsStart(void)
{
  //ReverseMutexGuard rGrd(_sync);
  return (_tcpSrv.Start() == TcpServerIface::rcOk) ? ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
}

void ICSTcpServer::_icsStop(bool do_wait/* = false*/)
{
  TcpServer::State_e res = TcpServer::srvIdle;
  {
    ReverseMutexGuard rGrd(_sync);
    res = _tcpSrv.Stop(do_wait);
  }
  if (res <= TcpServer::srvInited)
    _icsState = ICServiceAC::icsStInited;
  //else _tcpSrv doesn't stoppped
}

// --------------------------------------------------------------------------
// -- ICSTcpServerIface interface mthods
// --------------------------------------------------------------------------
//Registers an ICService providing specified protocol functionality.
//Returns false if given protocol is already provided by registered ICService.
bool ICSTcpServer::registerProtocol(ICSConnServiceIface & conn_srv)
{
  MutexGuard  grd(_sync);
  const IProtocolId_t & protoId = conn_srv.protoDef().ident();
  ProtocolsMap::const_iterator cit = _protoReg.find(protoId);
  if (cit != _protoReg.end()) {
    smsc_log_warn(logger, "%s: secondary attempt to register protocol %s",
                  _logId, protoId.c_str());
    return false;
  }
  _protoReg.insert(ProtocolsMap::value_type(protoId, ProtocolInfo(&conn_srv)));
  return true;
}

//Unregisters an ICService providing specified protocol functionality.
//Returns false if ICSTcpServer has set a reference to given ICService, 
//so it cann't be unregistered right now.
bool ICSTcpServer::unregisterProtocol(const IProtocolId_t & proto_id)
{
  MutexGuard  grd(_sync);
  ProtocolsMap::iterator it = _protoReg.find(proto_id);
  if (it == _protoReg.end()) {
    smsc_log_warn(logger, "%s: attempt to unregister unknown protocol %s",
                  _logId, proto_id.c_str());
  } else {
    if (it->second._refs)
      return false;
    _protoReg.erase(it);
  }
  return true;
}

// --------------------------------------------------------------------------
// -- TcpServerListenerIface interface mthods
// --------------------------------------------------------------------------
SocketListenerIface *
    ICSTcpServer::onConnectOpening(TcpServerIface & p_srv, ConnectUId conn_id,
                                   std::auto_ptr<Socket> & use_sock)
{
  MutexGuard  grd(_sync);
  //cleanUp died connects first
  _corpses.clear();

  ConnectInfo newConn;
  newConn._grd = _connPool.allcObj();
  //set consequtive processing of incoming packets
  newConn._grd->init(conn_id, _pckPool, 1, logger);
  newConn._grd->bind(use_sock);
  //listen for 1st incoming packet in order to detect required protocol
  newConn._grd->addListener(*this);
  if (newConn._grd->start()) {
    _connMap.insert(ConnectsMap::value_type(newConn._grd->getUId(), newConn));
    smsc_log_info(logger, "%s: activated %s", _logId, newConn._grd->logId());
    return newConn._grd.get();
  }
  smsc_log_fatal(logger, "%s: failed to start %s", _logId, newConn._grd->logId());
  return NULL;
}

//Notifies that connection is to be closed on given soket, no more events will be reported.
void ICSTcpServer::onConnectClosing(TcpServerIface & p_srv, ConnectUId conn_id)
{
  MutexGuard grd(_sync);
  //cleanUp died connects first
  _corpses.clear();

  ConnectsMap::iterator it = _connMap.find(conn_id);
  if (it == _connMap.end()) //Connect already unregistered by onConnectError()
    return;

  ConnectInfo rConn = it->second;

  it->second._grd->removeListener(*this);
  //Note: Connect object MUST not be destroyed inside this thread,
  //so postpone its destruction.
  _corpses.push_back(it->second._grd);
  _connMap.erase(it);

  if (rConn._protoId.empty()) { //no protocol was assigned
    smsc_log_info(logger, "%s: unregistered %s, protocol <not assigned>",
                  _logId, rConn._grd->logId());
    return;
  }

  smsc_log_info(logger, "%s: unregistered %s, protocol %s",
                _logId, rConn._grd->logId(), rConn._protoId.c_str());

  //Check if registered services should be notified
  ProtocolsMap::iterator cit = _protoReg.find(rConn._protoId);
  if (cit != _protoReg.end()) {
    ++(cit->second._refs);
    {
      ReverseMutexGuard rGrd(_sync);
      cit->second._srv->onDisconnect(rConn._grd);
    }
    --(cit->second._refs);
  }
}

//notifies that TcpServer is shutdowned, no more events on any connect will be reported.
void ICSTcpServer::onServerShutdown(TcpServerIface & p_srv, TcpServerIface::RCode_e down_reason)
{
  _tcpSrv.removeListener(*this);
  MutexGuard  grd(_sync);
  //cleanUp died connects first
  _corpses.clear();

  _icsState = ICServiceAC::icsStInited;
  smsc_log_debug(logger, "%s: TCP server shutdowned, reason %d", _logId, down_reason);
}

// --------------------------------------------------------------------------
// -- PacketListenerIface interface mthods
// --------------------------------------------------------------------------
//Returns true if listener has utilized packet so no more listeners
//should be notified, false - otherwise (in that case packet will be
//reported to other listeners).
bool ICSTcpServer::onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
  /*throw(std::exception) */
{
  MutexGuard  grd(_sync);
  ConnectsMap::iterator it = _connMap.find(conn_id);
  if (it == _connMap.end()) { //internal inconsistency
    smsc_log_warn(logger, "%s: packet is received on unregistered Connect[%u]",
                  _logId, conn_id);
    return true;
  }

  ConnectGuard  connGrd = it->second._grd;
  connGrd->removeListener(*this);

  ProtocolsMap::iterator pIt;
  if (!detectProto(recv_pck, pIt)) {
    smsc_log_warn(logger, "%s: unknown packet is received on %s",
                  _logId, connGrd->logId());
    _tcpSrv.rlseConnectionNotify(conn_id, true);
    return true;
  }

  ICSConnServiceIface * connSrv = pIt->second._srv;
  bool isAccepted = false;
  ++(pIt->second._refs);
  {
    ReverseMutexGuard rGrd(_sync);
    isAccepted = connSrv->setConnListener(connGrd);
  }
  --(pIt->second._refs);

  if (!isAccepted) {
    smsc_log_warn(logger, "%s: %s service provider denied %s",
                  _logId, connSrv->protoDef().ident().c_str(), connGrd->logId());
    _tcpSrv.rlseConnectionNotify(conn_id, true);
  } else {
    it = _connMap.find(conn_id); //NOTE: EOF event may be already in process
    if (it != _connMap.end())
      it->second._protoId = connSrv->protoDef().ident();
    smsc_log_info(logger, "%s: %s service provider accepted %s",
                  _logId, connSrv->protoDef().ident().c_str(), connGrd->logId());
  }
  return !isAccepted; //if ok, report packet to newly created connect listener
}

//Returns true if listener has processed connect exception so no more
//listeners should be notified, false - otherwise (in that case exception
//will be reported to other listeners).
bool ICSTcpServer::onConnectError(unsigned conn_id,
                                  PckAccumulatorIface::Status_e err_status,
                                  const CustomException * p_exc/* = NULL*/)
  /*throw(std::exception) */
{
  if ((err_status != PckAccumulatorIface::accEOF) || p_exc) {
    smsc_log_error(logger, "%s: Connect[%u] error status(%u): %s", _logId, conn_id,
                   (unsigned)err_status, p_exc ? p_exc->what() : "");
  }
  MutexGuard  grd(_sync);
  ConnectsMap::iterator it = _connMap.find(conn_id);
  if (it == _connMap.end()) //Connect already unregistered by onConnectClosing()
    return true;

  it->second._grd->removeListener(*this);
  //Note: Connect object MUST not be destroyed inside this method,
  //so postpone its destruction.
  _corpses.push_back(it->second._grd);
  _connMap.erase(it);
  smsc_log_info(logger, "%s: unregistered Connect[%u], protocol <unknown>",
                _logId, conn_id);
  return true;
}

// --------------------------------------------------------------------------
// -- Private mthods
// --------------------------------------------------------------------------
//Attempts to detect protocol the received packet belongs to.
//Return false in case of failure.
bool ICSTcpServer::detectProto(const PacketBufferAC & recv_pck,
                               ProtocolsMap::iterator & out_cit)
{
  for (out_cit = _protoReg.begin(); out_cit != _protoReg.end(); ++out_cit) {
    if (out_cit->second._srv->protoDef().isKnownPacket(recv_pck))
      return true;
  }
  return false;
}

} //tcpsrv
} //inman
} //smsc

