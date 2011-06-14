/* ************************************************************************** *
 * Generic interface of socket incoming events listener.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_SOCKET_LISTENER_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_SOCKET_LISTENER_DEFS_HPP

#include <errno.h>
#include <memory>

#include "core/network/Socket.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

using smsc::core::network::Socket;

class SocketListenerIface {
public: 
  enum ConnectionState_e { connAlive = 0, connEOF, connException };

private:
  SOCKET                      _hdlId;

protected:
  volatile ConnectionState_e  _connState;
  std::auto_ptr<Socket>       _socket;


  SocketListenerIface() : _hdlId(INVALID_SOCKET), _connState(connEOF)
  { }
  explicit SocketListenerIface(std::auto_ptr<Socket> & use_sock)
    : _hdlId(use_sock->getSocket())
    , _connState(use_sock->isOpened() ? connAlive : connEOF)
  {
    _socket.reset(use_sock.release());
  }
  //
  virtual ~SocketListenerIface()
  { }

  void assignSocket(std::auto_ptr<Socket> & use_sock)
  {
    _hdlId = use_sock->getSocket();
    _connState = use_sock->isOpened() ? connAlive : connEOF;
    _socket.reset(use_sock.release());
  }

public:
  SOCKET    getId(void)  const { return _hdlId; }
  bool      isOpened(void) const { return _socket.get() && _socket->isOpened(); }
  Socket *  getSocket(void) const { return _socket.get(); }

  ConnectionState_e getState(void) const { return _connState; };

  // ---------------------------------------------------
  // -- SocketListenerIface interface methods
  // ---------------------------------------------------
  //Notifies that either data is available for reading or normal EOF condition
  //is reached on socket.
  //Further connection state depends on returned status.
  virtual ConnectionState_e onReadEvent(void) /*throw()*/= 0;
  //Notifies that socket is to be closed or aborted due to reason identified
  //by 'err_no' argument.  Zero value of 'err_no' means socket is normally
  //closed by request of one of peers.
  virtual void onCloseEvent(int err_no) /*throw()*/= 0;
};
typedef SocketListenerIface::ConnectionState_e ConnectState_e;

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_SOCKET_LISTENER_DEFS_HPP */

