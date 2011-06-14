/* ************************************************************************* *
 * TCP Server service interface definitions.
 * ************************************************************************* */
#ifndef __INMAN_ICS_TCPSERVER_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_TCPSERVER_DEFS_HPP

#include "inman/interaction/IProtocol.hpp"
#include "inman/interaction/asyncmgr/ConnectsPool.hpp"

namespace smsc {
namespace inman {
namespace tcpsrv {

using smsc::inman::interaction::IProtocolAC;
using smsc::inman::interaction::IProtocolId_t;
using smsc::inman::interaction::ConnectGuard;

class ICSConnServiceIface {
protected:
  virtual ~ICSConnServiceIface() //forbid interface destruction
  { }

public:
  //Returns definition of IProtocol this service provides
  virtual const IProtocolAC & protoDef(void) const = 0;
  //Creates a connect listener serving given connect.
  //Returns true on success, false -  otherwise.
  //Note: upon entry the referenced Connect is configured 
  //  to process in consequitive mode, so it's recommended
  //  to reconfigure Connect within this call.
  virtual bool setConnListener(const ConnectGuard & use_conn) /*throw()*/= 0;
  //Notifies that given connection is to be closed, no more socket events will be reported.
  virtual void onDisconnect(const ConnectGuard & use_conn) /*throw()*/= 0;
};


class ICSTcpServerIface {
protected:
  virtual ~ICSTcpServerIface() //forbid interface destruction
  { }

public:
  //Registers an ICService providing specified protocol functionality.
  //Returns false if given protocol is already provided by registered ICService.
  virtual bool registerProtocol(ICSConnServiceIface & conn_srv) = 0;
  //Attempts to unregister an ICService providing specified protocol functionality.
  //Returns false if ICSTcpServer set a reference to given ICService.
  //so it cann't be unregistered right now.
  virtual bool unregisterProtocol(const IProtocolId_t & proto_id) = 0;
};

} //tcpsrv
} //inman
} //smsc
#endif /* __INMAN_ICS_TCPSERVER_DEFS_HPP */

