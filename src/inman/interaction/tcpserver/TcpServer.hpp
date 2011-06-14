/* ************************************************************************** *
 * TCP Server: manages client's connection requests, listens for sockets,
 * notifies associated connect listeners if data arrives.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_TCP_SERVER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TCP_SERVER_HPP

#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"

#include "inman/common/RFCPList_T.hpp"
#include "inman/interaction/tcpserver/TcpServerCfg.hpp"
#include "inman/interaction/tcpserver/SocketsRegistry.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

using smsc::logger::Logger;
using smsc::core::threads::Thread;
using smsc::core::synchronization::Condition;
using smsc::core::synchronization::Event;
using smsc::core::synchronization::EventMonitor;


class TcpServer : public TcpServerIface, protected Thread,
                  private SocketListenerIface {
public:
  static const char * _DFLT_IDENT; //"TCPSrv"
  static const unsigned int _DFLT_SHUTDOWN_TMO_MS = 400; //millisecs

  explicit TcpServer(const char * use_id = _DFLT_IDENT, Logger * use_log = NULL)
    : TcpServerIface(use_id), _runState(TcpServer::srvIdle), _lsrList(&_sync)
    , _connReg(2), _logger(use_log ? use_log : Logger::getInstance("smsc.inman"))
  {
    if (_idStr.empty())
      _idStr = _DFLT_IDENT;
  }
  //
  virtual ~TcpServer()
  {
    Release();
  }

  //Initializes the server as a registered sockets listener only.
  //Incoming connection requests cann't be handled.
  //Switches FSM to srvInited.
  RCode_e Init(const TcpListenerCFG & use_cfg);
  //Fully initializes the server (incoming connection requests are served on
  //on given socket). Switches FSM to srvInited.
  RCode_e Init(const TcpServerCFG & use_cfg);
  //Starts socket listening thread and switches FSM to srvRunning.
  RCode_e Start(void);
  //Stops socket listening thread switching FSM either to srvStopping or srvInited
  //depending on specified 'do_wait' value.
  //If 'do_wait' is true, this method blocks until listener thread will be completed.
  State_e Stop(bool do_wait = true);
  //Releases all allocated resources. Completely stops service if required (calls Stop(true);).
  void    Release(void);

  void setShutdownTimeout(unsigned int milli_secs);
  void setPollTimeout(unsigned int milli_secs);

  // -----------------------------------------
  // -- TcpServerIface interface methods
  // -----------------------------------------
  virtual State_e getState(void) const /*throw()*/;
  //Establishes outgoing connection to given host.
  //Returns true on success and initialized Socket object.
  virtual bool setConnection(const char * host, unsigned port,
                             std::auto_ptr<Socket> & conn_sock);
  //Registers already established connection and starts to control the
  //associated socket events. If 'monitoring_on' is TRUE, server listeners
  //will be notified about this connection closing.
  //Returns false if associated socket isn't connected or connection
  //is already registered.
  virtual bool registerConnection(SocketListenerIface & sock_hdl, bool monitoring_on = false);
  //Waits until last connection event is handled and closes/aborts socket.
  //No listeners are notified about socket closing.
  virtual void rlseConnectionWait(unsigned sock_id, bool do_abort = false);
  //Marks connection as subject for closing and returns immediately.
  //Registered listener(s) will be notified on socket closing.
  //Returns false if no connection is associated with given socket.
  virtual bool rlseConnectionNotify(unsigned sock_id, bool do_abort = false);
  //
  virtual void addListener(TcpServerListenerIface & use_lstr)
  {
    _lsrList.push_back(&use_lstr);
  }
  //
  virtual void removeListener(TcpServerListenerIface & use_lstr)
  {
    _lsrList.remove(&use_lstr);
  }

private:
  using Thread::Start; //Start() is overloaded

  // ---------------------------------------------------
  // -- SocketListenerIface interface methods
  // ---------------------------------------------------
  //called if EOF notification or data for reading arrived on socket,
  //connect may be closed depending on returned status.
  virtual ConnectionState_e onReadEvent(void) /*throw()*/;
  //Notifies that socket is to be closed due to reason identified by 'err_no'
  //argument. Zero value means socket is closed by some external request.
  virtual void onCloseEvent(int err_no) /*throw()*/;

protected:
  typedef smsc::util::RFCPListGuarded_T<TcpServerListenerIface> ListenersList;

  volatile State_e      _runState;
  mutable EventMonitor  _sync; //guards: _runState, _lsrList, _sockMap

  ListenersList         _lsrList;
  Condition             _connEvent; //guards connection register/unregister events
  SocketsRegistry       _connReg;   //active connections registry

  Event                 _lstEvent;
  TcpServerCFG          _cfg;
  Logger *              _logger;
  /* */

  RCode_e _initSrvSocket(void);
  RCode_e _startServerSocket(void);
  void    _listenSockets(void);

  //Notifies TcpServerlisteners about incoming connection request.
  //If some listener creates SocketEventHandler a new connection is registered.
  bool _openConnect(std::auto_ptr<Socket> & use_sock);
  //Returns true if connection is unregistered and doesn't more monitored.
  //Also if connection is still alive, returns its SocketInfo data.
  //Otherwise marks it as subject for closing.
  bool _unregisterConnection(unsigned sock_id, bool do_abort, SocketInfo * p_inf = NULL);
  void _notifyOnClose(const SocketInfo & conn_inf) /*throw()*/;
  //
  void _closeConnection(SocketsRegistry::iterator use_it) /*throw()*/;
  void _closeAllConnects(bool do_abort = true, bool do_notify = false);

  // ---------------------------------
  // -- Thread interface methods
  // ---------------------------------
  virtual int  Execute(); //listener thread entry point
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCP_SERVER_HPP */

