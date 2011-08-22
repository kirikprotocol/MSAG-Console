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
  typedef unsigned int ident_type;
  enum ConnectionState_e { connAlive = 0, connEOF, connException };

protected:
  ident_type                  _uId;
  volatile ConnectionState_e  _connState;
  std::auto_ptr<Socket>       _socket;


  explicit SocketListenerIface(ident_type use_uid)
    : _uId(use_uid), _connState(connEOF)
  { }
  SocketListenerIface(ident_type use_uid, std::auto_ptr<Socket> & use_sock)
    : _uId(use_uid)
    , _connState((use_sock.get() && use_sock->isOpened()) ? connAlive : connEOF)
  {
    _socket.reset(use_sock.release());
  }
  //
  virtual ~SocketListenerIface()
  { }

  void assignSocket(std::auto_ptr<Socket> & use_sock)
  {
    _socket.reset(use_sock.release());
    _connState = isOpened() ? connAlive : connEOF;
  }

public:
  //Returns unique listener id
  ident_type  getUId(void)  const { return _uId; }
  //Returns file descriptor associated with opened socket
  SOCKET      getFd(void)  const { return _socket.get() ? _socket->getSocket(): INVALID_SOCKET; }
  //Returns true if socket is assigned and is opened.
  bool        isOpened(void) const { return _socket.get() && _socket->isOpened(); }

  ConnectionState_e getState(void) const { return _connState; };
  //
  Socket *  getSocket(void) const { return _socket.get(); }

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
typedef SocketListenerIface::ConnectionState_e  ConnectState_e;
typedef SocketListenerIface::ident_type         ConnectUId;

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_SOCKET_LISTENER_DEFS_HPP */

