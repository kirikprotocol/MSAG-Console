#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/tcpserver/TcpServer.hpp"
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::ReverseMutexGuard;

namespace smsc  {
namespace inman {
namespace interaction  {

#define LISTENER_RESTART_ATTEMPTS   2
/* ************************************************************************** *
 * class TcpServer implementation:
 * ************************************************************************** */
const char * TcpServer::_DFLT_IDENT = "TCPSrv";

void TcpServer::setShutdownTimeout(unsigned int milli_secs)
{
  _cfg._shtdTmo = (milli_secs < TcpServerCFG::_MIN_SHUTDOWN_TMO_MS) ?
                  TcpServerCFG::_MIN_SHUTDOWN_TMO_MS : milli_secs;
}

void TcpServer::setPollTimeout(unsigned int milli_secs)
{
  if (milli_secs < TcpServerCFG::_MIN_POLL_TMO_MS)
    milli_secs = TcpServerCFG::_MIN_POLL_TMO_MS;

  _cfg._pollTmo = milli_secs;
}

//Initializes the server as a socket listener only, switches FSM to srvInited.
TcpServer::RCode_e TcpServer::Init(const TcpListenerCFG & use_cfg)
{
  MutexGuard  grd(_sync);
  if (_runState > TcpServerIface::srvIdle)
    return TcpServerIface::rcSrvState;

  _cfg = use_cfg;
  setPollTimeout(_cfg._pollTmo ? _cfg._pollTmo : TcpServerCFG::_MIN_POLL_TMO_MS);
  setShutdownTimeout(_cfg._shtdTmo ? _cfg._shtdTmo : TcpServer::_DFLT_SHUTDOWN_TMO_MS);
  _runState = TcpServerIface::srvInited;
  return TcpServer::rcOk;
}

TcpServer::RCode_e TcpServer::Init(const TcpServerCFG & use_cfg)
{
  MutexGuard  grd(_sync);
  if (_runState > TcpServerIface::srvIdle)
    return TcpServerIface::rcSrvState;

  _cfg = use_cfg;
  _connReg.reserve(_cfg._maxConn + 1);
  setPollTimeout(_cfg._pollTmo ? _cfg._pollTmo : TcpServerCFG::_MIN_POLL_TMO_MS);
  setShutdownTimeout(_cfg._shtdTmo ? _cfg._shtdTmo : TcpServer::_DFLT_SHUTDOWN_TMO_MS);
  return _initSrvSocket();
}

TcpServer::RCode_e TcpServer::Start(void)
{
  int rc = 0;
  RCode_e rval = TcpServerIface::rcOk;
  {
    MutexGuard  grd(_sync);
    if (_runState == TcpServerIface::srvRunning)
      return rval;
    if ((_runState == TcpServerIface::srvStopping)) {
      if ((rval = _startServerSocket()) == TcpServerIface::rcOk) {
        _runState = TcpServerIface::srvRunning;
        _sync.notify();
      }
      return rval;
    }
    if (_runState < TcpServerIface::srvInited) {
      if ((rval = _initSrvSocket()) != TcpServerIface::rcOk)
        return rval;
    }
    //here: srvInited
    if ((rval = _startServerSocket()) != TcpServerIface::rcOk)
      return rval;

    Thread::Start();
    rc = Thread::getRetCode();
  }
  if (rc) {
    smsc_log_fatal(_logger, "%s: Listener thread start is failed: %s (%d)",
                    getIdent(), strerror(rc), rc);
    return TcpServerIface::rcLstError;
  }
  if ((rc = _lstEvent.Wait(_cfg._pollTmo)) != 0) {
    smsc_log_error(_logger, "%s: Listener thread start is timed out: %s (%d)",
                    getIdent(), strerror(rc), rc);
    return TcpServerIface::rcLstError;
  }
  return rval;
}

TcpServer::State_e TcpServer::Stop(bool do_wait)
{
  {
    MutexGuard  tmp(_sync);
    if (_runState <= TcpServerIface::srvInited)
      return _runState;

    _runState = TcpServerIface::srvStopping;
    if (!do_wait) {
      smsc_log_debug(_logger, "%s: stopping Listener ..", getIdent());
      return _runState;
    }
    smsc_log_debug(_logger, "%s: stopping Listener, timeout = %u ms ..",
                    getIdent(), _cfg._shtdTmo);
  }
  _lstEvent.Wait(_cfg._shtdTmo);
  {
    MutexGuard  tmp(_sync);
    if (_runState > TcpServerIface::srvInited) {
      _runState = TcpServerIface::srvInited;  //forcedly stop Listener thread
      smsc_log_debug(_logger, "%s: timeout expired, %u connects are still active",
                      getIdent(), _connReg.size());
    }
  }
  Thread::WaitFor();
  return _runState;
}

void TcpServer::Release(void)
{
  Stop(true);

  MutexGuard grd(_sync);
  _closeAllConnects(true, false);
  _runState = TcpServerIface::srvIdle;
}

// -----------------------------------------
// -- TcpServerIface interface methods
// -----------------------------------------
TcpServer::State_e TcpServer::getState(void) const /*throw()*/
{
  MutexGuard  grd(_sync);
  return _runState;
}

ConnectUId TcpServer::setConnection(const char * host, unsigned port, std::auto_ptr<Socket> & conn_sock)
{
  conn_sock.reset(new Socket());
  if (conn_sock->Init(host, port, _cfg._pollTmo*1000) || conn_sock->Connect()) {
    smsc_log_error(_logger, "%s: unable to set connection to %s:%u : %s (%d)", getIdent(),
                   host, port, strerror(errno), errno);
    conn_sock.reset();
    return false;
  }

  smsc_log_debug(_logger, "%s: Socket[%u] is connected to %s:%u", getIdent(),
                 conn_sock->getSocket(), host, port);

  MutexGuard  grd(_sync);
  ConnectUId connId = getNextConnId();
  if (!connId) {
    smsc_log_warn(_logger, "%s: unable to allocate ConnectUId, closing Socket[%u]", getIdent(),
                  conn_sock->getSocket());
    conn_sock.reset();
  }
  return connId;
}

bool TcpServer::registerConnection(SocketListenerIface & sock_hdl, bool monitoring_on/* = false*/)
{
  MutexGuard  grd(_sync);
  if (!sock_hdl.isOpened()) {
    smsc_log_error(_logger, "%s: attempt to register uninitialized Connect", getIdent());
    return false;
  }

  SocketsRegistry::const_iterator cit = _connReg.find(sock_hdl.getUId());
  if (cit != _connReg.end()) {
    smsc_log_error(_logger, "%s: Connect[%u] is already registered", getIdent(), sock_hdl.getUId());
    return false;
  }
  _connReg.insert(SocketInfo(sock_hdl, monitoring_on));
  smsc_log_info(_logger, "%s: Connect[%u]{%u} is registered", getIdent(),
                sock_hdl.getUId(), sock_hdl.getFd());
  _sync.notify(); //awake _listenSockets()
  return true;
}

void TcpServer::rlseConnectionWait(ConnectUId conn_id, bool do_abort/* = false*/)
{
  SocketInfo  connInf;
  {
    MutexGuard  grd(_sync);
    while (!_unregisterConnection(conn_id, do_abort, &connInf))
      _connEvent.WaitOn(_sync);
  }
  if (!connInf.empty()) {
    smsc_log_info(_logger, "%s: %s Connect[%u]{%u}", getIdent(),
                  do_abort ? "aborting" : "closing", conn_id, connInf.getFd());
    connInf.closeSocket();
  }
}

bool TcpServer::rlseConnectionNotify(ConnectUId conn_id, bool do_abort/* = false*/)
{
  MutexGuard  grd(_sync);
  SocketsRegistry::iterator it = _connReg.find(conn_id);
  if (it != _connReg.end()) {
    it->second.markToClose(do_abort, true);
    return true;
  }
  return false;
}

/* -------------------------------------------------------------------------- *
 * Private/Protected methods:
 * -------------------------------------------------------------------------- */
#define MAX_CONN_UID_ATTEMPT 3
//Returns zero if no UId available.
ConnectUId TcpServer::getNextConnId(void)
{
  ConnectUId rval = 0;
  unsigned short numAtt = 0;

  do {
    if (!(rval = ++_lastConnId))
      rval = ++_lastConnId;

    SocketsRegistry::const_iterator it = _connReg.find(rval);
    if (it == _connReg.end())
      return rval;

  } while (++numAtt < MAX_CONN_UID_ATTEMPT);
  
  return 0;
}
// ---------------------------------------------------
// -- SocketListenerIface interface methods
// ---------------------------------------------------
ConnectState_e TcpServer::onReadEvent(void) /*throw()*/
{
  MutexGuard  grd(_sync);
  std::auto_ptr<Socket> clientSocket(_socket->Accept());

  if (!clientSocket.get()) {
    smsc_log_error(_logger, "%s: failed to accept client connection, cause: %d (%s)",
                    getIdent(), errno, strerror(errno));
    return SocketListenerIface::connAlive;
  } 
  if (_runState != srvRunning) {
    smsc_log_warn(_logger, "%s: connection refused, stopping ..", getIdent());
    return SocketListenerIface::connAlive;
  }
  if (_connReg.size() >= (_cfg._maxConn + 1)) {
    smsc_log_warn(_logger, "%s: connection refused, "
                  "resource limitation: %u", getIdent(), _cfg._maxConn);
  } else {
    smsc_log_info(_logger, "%s: accepted connection on Socket[%u]", getIdent(),
                   clientSocket->getSocket());
    _openConnect(clientSocket);
  }
  return SocketListenerIface::connAlive;
}

void TcpServer::onCloseEvent(int err_no) /*throw()*/
{
  MutexGuard  grd(_sync);
  if (err_no) {
    smsc_log_fatal(_logger, "%s: Error on server socket : %d (%s)",
                   getIdent(), err_no, strerror(err_no));
    _runState = TcpServerIface::srvStopping;
  } else {
    smsc_log_info(_logger, "%s: Closing server socket ..", getIdent());
  }
  SocketsRegistry::iterator it = _connReg.find(SocketListenerIface::getUId());
  SocketInfo & rInf = it->second;
  rInf.setRef();
  rInf.markToClose(true, false);
  _closeConnection(it);
  Thread::setRetCode(TcpServerIface::rcSrvError);
}


/* -------------------------------------------------------------------------- *
 * NOTE: _sync MUST BE LOCKED upon entry to following methods:
 * -------------------------------------------------------------------------- */
//Switches FSM: srvIdle -> srvInited
TcpServer::RCode_e TcpServer::_initSrvSocket(void)
{
  if (_cfg._host.empty())
    return TcpServerIface::rcOk;

  std::auto_ptr<Socket> srvSocket(new Socket());

  if (srvSocket->InitServer(_cfg._host.c_str(), _cfg._port, _cfg._pollTmo*1000, 1, true)) {
    smsc_log_fatal(_logger, "%s: failed to init service socket at %s:%u : %d (%s)",
                   getIdent(), _cfg._host.c_str(), _cfg._port, errno, strerror(errno));
    return TcpServerIface::rcSrvError;
  }
  SocketListenerIface::assignSocket(srvSocket); //connId = 0
  _connReg.insert(SocketInfo(*(SocketListenerIface*)this, false));
  smsc_log_info(_logger, "%s: opened service Socket[%u] at %s:%u", getIdent(),
                 SocketListenerIface::getFd(), _cfg._host.c_str(), _cfg._port);

  _runState = TcpServerIface::srvInited;
  return TcpServerIface::rcOk;
}

TcpServer::RCode_e TcpServer::_startServerSocket(void)
{
  if (!_cfg._host.empty()) {
    if (!_socket->isConnected() && _socket->StartServer()) {
      smsc_log_fatal(_logger, "%s: failed to start server on Socket[%u] at %s:%d",
                     getIdent(), SocketListenerIface::getFd(),
                     _cfg._host.c_str(), _cfg._port);
      return TcpServerIface::rcSrvError;
    }
    smsc_log_info(_logger, "%s: started server on Socket[%u] at %s:%u", getIdent(),
                   SocketListenerIface::getFd(), _cfg._host.c_str(), _cfg._port);
  }
  return TcpServerIface::rcOk;
}

//Returns true if connection is unregistered and no more monitored.
//Also if connection is still alive, returns its SocketInfo data.
//Otherwise marks it as subject for closing.
bool TcpServer::_unregisterConnection(unsigned sock_id, bool do_abort, SocketInfo * p_inf/* = NULL*/)
{
  if (p_inf)
    p_inf->clear();

  SocketsRegistry::iterator it = _connReg.find(sock_id);
  if (it == _connReg.end())
    return true;
  
  SocketInfo & rInf = it->second;
  if (!rInf.isInUse()) {
    if (rInf.sockHandler()->isOpened() && p_inf)
      *p_inf = rInf;
    _connReg.erase(it);
    return true;
  }
  rInf.markToClose(do_abort, false);
  return false;
}

//Notifies TcpServerListener(s) about connection closing
void TcpServer::_notifyOnClose(const SocketInfo & conn_inf) /*throw()*/
{
  if (conn_inf.needNotify() && conn_inf.isMonitored()) {
    unsigned connId = conn_inf.sockHandler()->getUId();
    try {
      for (ListenersList::iterator it = _lsrList.begin(); !it.isEnd(); ++it) {
        it->onConnectClosing(*this, connId);
      }
    } catch (const std::exception & exc) {
      smsc_log_error(_logger, "%s: Connect[%u] handler exception (onConnectClosing): %s",
                      getIdent(), connId, exc.what());
    } catch (...) {
      smsc_log_error(_logger, "%s: Connect[%u] handler exception (onConnectClosing): <unknown>",
                      getIdent(), connId);
    }
  }
}

void TcpServer::_closeConnection(SocketsRegistry::iterator use_it) /*throw()*/
{
  SocketInfo connInf = use_it->second;
  _connReg.erase(use_it);
  {
    ReverseMutexGuard rGrd(_sync);
    smsc_log_info(_logger, "%s: %s Connect[%u]{%u}", getIdent(), 
                  connInf.needAbort() ? "aborting" : "closing", connInf.getUId(), connInf.getFd());
    connInf.closeSocket();
    _notifyOnClose(connInf);
  }
  _connEvent.SignalAll();
}

void TcpServer::_closeAllConnects(bool do_abort/* = true*/, bool do_notify/* = false*/)
{
  if (!_connReg.empty()) {
    smsc_log_debug(_logger, "%s: %s %u connects ..",  getIdent(),
                   do_abort ? "aborting" : "closing", _connReg.size());
    do {
      SocketInfo & rInf = _connReg.begin()->second;
      rInf.setRef();
      rInf.markToClose(do_abort, do_notify);
      _closeConnection(_connReg.begin());
    } while (!_connReg.empty());
  }
}
//Notifies TcpServerlisteners about incoming connection request.
//If listener creates SocketEventHandler a new connection is registered.
bool TcpServer::_openConnect(std::auto_ptr<Socket> & use_sock) /*throw()*/
{
  unsigned  sockId = use_sock->getSocket();
  SocketListenerIface * sockHdl = NULL;

  use_sock->SetNoDelay(true);
  ConnectUId connId = getNextConnId();

  if (!connId) {
    smsc_log_warn(_logger, "%s: unable to allocate ConnectUId", getIdent());
  } else {
    try {
      ReverseMutexGuard rGrd(_sync);
      //notify registered TcpServerListeners about connect creation
      for (ListenersList::iterator it = _lsrList.begin(); !it.isEnd() && use_sock.get(); ++it) {
        sockHdl = it->onConnectOpening(*this, connId, use_sock);
      }
    } catch (const std::exception & exc) {
      smsc_log_error(_logger, "%s: TcpServerListener exception (onConnectOpening): %s",
                      getIdent(), exc.what());
    } catch (...) {
      smsc_log_error(_logger, "%s: TcpServerListener exception (onConnectOpening): <unknown>",
                      getIdent());
    }
  }

  if (sockHdl && (sockHdl->getState() == SocketListenerIface::connAlive)) {
    _connReg.insert(SocketInfo(*sockHdl, true));
    smsc_log_info(_logger, "%s: Connect[%u]{%u} is registered", getIdent(),
                  sockHdl->getUId(), sockHdl->getFd());
    _sync.notify(); //awake _listenSockets()
    return true;
  }
  use_sock->Close();
  smsc_log_warn(_logger, "%s: Socket[%u] closed: no listener set!", getIdent(), sockId);
  return false;
}

void TcpServer::_listenSockets(void)
{
  _sync.Lock();
  _runState = TcpServerIface::srvRunning;

  while (_runState > TcpServerIface::srvInited) {
    if (_runState == TcpServerIface::srvStopping) {
      //close service socket, other connects will be served for a while
      if (SocketListenerIface::isOpened()) {
        smsc_log_debug(_logger, "%s: stopping service Socket[%u] ..", getIdent(), SocketListenerIface::getFd());
        SocketsRegistry::iterator it = _connReg.find(SocketListenerIface::getUId());
        if (it == _connReg.end()) {
          smsc_log_warn(_logger, "%s: service Socket[%u] isn't registered", getIdent(), SocketListenerIface::getFd());
        } else
          it->second.markToClose(false, false);
      }
    }

    //traverse Connects registry: mark active, close inactive ones.
    SocketsRegistry::iterator nit = _connReg.begin();
    while (nit != _connReg.end()) {
      SocketsRegistry::iterator it = nit++;
      SocketInfo & rInf = it->second;
      rInf.setRef();
      if (rInf.isToClose())
        _closeConnection(it); //erases all connect info/data at this position
    }

    int         n = 0;
    pollfd_arr  fdArr;

    if (_connReg.empty()) {
      if (_runState < TcpServerIface::srvRunning) {
        _runState = TcpServerIface::srvInited;
        smsc_log_debug(_logger, "%s: all connects finished, stopping ..", getIdent());
        break;
      }
      _sync.wait(_cfg._pollTmo);
    } else {
      fdArr = _connReg.composeFds(); //here fd array refers only marked sockets!
      {
        ReverseMutexGuard rGrd(_sync);
        n = poll(fdArr._ptr, fdArr._size, _cfg._pollTmo);
      }
    }
    if (!n) //timeout expired, no data arrived to any osocket
      continue;
    if (n < 0) {
      if (errno == EINTR) {
        smsc_log_warn(_logger, "%s: poll() failed, cause: %d (%s)", getIdent(),
                       errno, strerror(errno));
      } else {
        smsc_log_fatal(_logger, "%s: poll() failed, cause: %d (%s)", getIdent(),
                       errno, strerror(errno));
        _runState = TcpServerIface::srvInited;
        Thread::setRetCode(TcpServerIface::rcSrvError);
      }
    } else {
      //traverse Connects registry: close inactive ones, serve poll events, unmark all
      nfds_t fdPos = 0;
      nit = _connReg.begin();
      while (nit != _connReg.end()) {
        SocketsRegistry::iterator it = nit++;
        SocketInfo & rInf = it->second;
        if (rInf.isToClose()) {
          _closeConnection(it); //erases all connect info/data at this position
          continue;
        }
        if (rInf.isInUse()) {
          //Scan pollfd array for occured events for this fd
          if (fdArr.find((int)rInf.getFd(), fdPos)) {
            short rEvents = fdArr._ptr[fdPos].revents;
            /* - */
            if (rEvents & POLLIN) {
              ConnectState_e  st = SocketListenerIface::connAlive;
              {
                ReverseMutexGuard rGrd(_sync);
                st = rInf.sockHandler()->onReadEvent();
              }
              if (st == SocketListenerIface::connEOF) {
                smsc_log_debug(_logger, "%s: handler ends Connect[%u]", getIdent(), rInf.getUId());
              }
              if (st != SocketListenerIface::connAlive) {
                rInf.markToClose(st != SocketListenerIface::connEOF, true);
                _closeConnection(it); //erases all connect info/data at this position
                continue;
              }
            }
            if (rEvents & (POLLNVAL | POLLERR | POLLHUP)) {
              smsc_log_error(_logger, "%s: error event on Socket[%u], mask 0x%X",
                             getIdent(), rInf.getFd(), rEvents);
              int errNum = EBADFD; /* f.d. invalid for this operation	*/
              if (rEvents & POLLHUP)
                errNum = EPIPE;
              else if (rEvents & POLLERR)
                errNum = EIO;
              {
                ReverseMutexGuard rGrd(_sync);
                rInf.sockHandler()->onCloseEvent(errNum);
              }
              rInf.markToClose(true, false);
              _closeConnection(it); //erases all connect info/data at this position
              continue;
            }
          }
        }
        rInf.unRef();
      }
    }
  } /* eow */
  _sync.Unlock();
}

// Thread entry point
int TcpServer::Execute()
{
  {
    MutexGuard  grd(_sync);
    Thread::setRetCode(TcpServerIface::rcOk);
    _lstEvent.Signal();
    smsc_log_debug(_logger, "%s: sockets listener started.", getIdent());
    try {
      ReverseMutexGuard rGrd(_sync);
      _listenSockets();
      /* */
    } catch (const std::exception & error) {
      smsc_log_error(_logger, "%s: sockets listener unexpected failure: %s", getIdent(), error.what());
      Thread::setRetCode(TcpServerIface::rcLstError);
      /* */
    } catch (...) {
      smsc_log_error(_logger, "%s: sockets listener unexpected exception", getIdent());
      Thread::setRetCode(TcpServerIface::rcLstError);
    }
    smsc_log_debug(_logger, "%s: sockets listener finished, cause %d", getIdent(), Thread::getRetCode());

    if (_runState > TcpServerIface::srvStopping) //exception arised in _listenSockets();
      _runState = TcpServerIface::srvStopping;

    _closeAllConnects(true, true); //abort active connects
    _runState = TcpServerIface::srvIdle;
    _lstEvent.Signal();
  }

  RCode_e downReason = static_cast<RCode_e>(Thread::getRetCode());
  //notify TcpServerListeners about server shutdown
  for (ListenersList::iterator it = _lsrList.begin(); !it.isEnd(); ++it)
    it->onServerShutdown(*this, downReason);
  return Thread::getRetCode();
}

} // namespace interaction
} // namespace inman
} // namespace smsc

