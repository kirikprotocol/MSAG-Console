/* ************************************************************************** *
 * TCP Server interoperation interfaces definitions
 * ************************************************************************** */
#ifndef __SMSC_INMAN_TCP_SERVER_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TCP_SERVER_DEFS_HPP

#include "core/buffers/FixedLengthString.hpp"
#include "inman/interaction/SocketListenerDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

class TcpServerListenerIface; //forward declaration

class TcpServerIface {
public:
  enum State_e {
    srvIdle = 0
    , srvInited   //service socket is properly initialized, sockets listener is inactive
    , srvStopping //sockets listener is stopping, no new incoming connects are accepted.
    , srvRunning  //sockets listener is active and accepts new incoming connects.
  };
  enum RCode_e {
    rcOk = 0      //
    , rcSrvState  //inconsistent server state
    , rcSrvError  //server socket fatal error
    , rcLstError  //sockets listener caught unexpected fatal exception
  };

  static const unsigned int _max_IDENT_LEN = 32;

  typedef smsc::core::buffers::FixedLengthString<_max_IDENT_LEN + 1> IdentString_t;

  const char * getIdent(void) const { return _idStr.empty() ? NULL : _idStr.c_str(); }

  // -----------------------------------------
  // -- TcpServerIface interface methods
  // -----------------------------------------
  virtual State_e getState(void) const /*throw()*/= 0;
  //Establishes outgoing connection to given host.
  //Returns initialized Socket object and connection Id reserved for it
  //in case of success, otherwise returns zero.
  virtual ConnectUId setConnection(const char * host, unsigned port,
                             std::auto_ptr<Socket> & conn_sock) = 0;
  //Registers already established connection and starts to control the
  //associated socket events. If 'monitoring_on' is TRUE, server listeners
  //will be notified about this connection closing.
  //Returns false if associated socket isn't connected or connection
  //is already registered.
  virtual bool registerConnection(SocketListenerIface & sock_hdl, bool monitoring_on = false) = 0;
  //Waits until last connection event is handled and closes/aborts socket.
  //No listeners are notified about socket closing.
  virtual void rlseConnectionWait(ConnectUId conn_id, bool do_abort = false) = 0;
  //Marks connection as subject for closing and returns immediately.
  //Registered listener(s) will be notified on socket closing.
  //Returns false if no connection  with given id is registered.
  virtual bool rlseConnectionNotify(ConnectUId conn_id, bool do_abort = false) = 0;
  //
  virtual void addListener(TcpServerListenerIface & use_lstr) /*throw()*/= 0;
  //
  virtual void removeListener(TcpServerListenerIface & use_lstr) /*throw()*/= 0;

protected:
  IdentString_t _idStr;
  ConnectUId    _lastConnId;

  explicit TcpServerIface(const char * use_id = NULL) : _idStr(use_id), _lastConnId(0)
  { }
  //
  virtual ~TcpServerIface() //forbid interface destruction
  { }
};

typedef TcpServerIface::State_e TcpSrvState_e;
typedef TcpServerIface::RCode_e TcpSrvRCode_e;
typedef TcpServerIface::IdentString_t TcpSrvIdent_t;

//
class TcpServerListenerIface {
protected:
  virtual ~TcpServerListenerIface() //forbid interface destruction
  { }

public:
  // ----------------------------------------------
  // -- TcpServerListenerIface interface methods:
  // ----------------------------------------------
  //Notifies that incoming connection with remote peer is accepted on given
  //socket.  If listener ignores connection, the 'use_sock' argument must be
  //kept intact and NULL must be returned.
  virtual SocketListenerIface *
    onConnectOpening(TcpServerIface & p_srv, ConnectUId conn_id, std::auto_ptr<Socket> & use_sock) = 0;
  //Notifies that given connection is to be closed, no more events will be reported.
  virtual void onConnectClosing(TcpServerIface & p_srv, ConnectUId conn_id) = 0;
  //notifies that TcpServer is shutdowned, no more events on any connect will be reported.
  virtual void onServerShutdown(TcpServerIface & p_srv, TcpServerIface::RCode_e down_reason) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCP_SERVER_DEFS_HPP */

