/* ************************************************************************** *
 * USS Gateway Service
 * ************************************************************************** */
#ifndef _SMSC_USS_GATEWAY_SERVICE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define _SMSC_USS_GATEWAY_SERVICE_HPP

#include "inman/inap/dispatcher.hpp"
#include "inman/interaction/tcpserver/TcpServer.hpp"

#include "ussman/service/USSGCfgDefs.hpp"
#include "ussman/service/USSConnManager.hpp"

namespace smsc  {
namespace ussman {

using smsc::logger::Logger;

using smsc::util::CustomException;
using smsc::core::network::Socket;

using smsc::inman::interaction::ConnectUId;
using smsc::inman::interaction::ConnectsPool;
using smsc::inman::interaction::ConnectGuard;
using smsc::inman::interaction::TcpServerCFG;
using smsc::inman::interaction::PacketBufferAC;
using smsc::inman::interaction::PckAccumulatorIface;
using smsc::inman::interaction::SocketListenerIface;
using smsc::inman::interaction::TcpServerIface;


class USSGService : protected smsc::inman::interaction::TcpServerListenerIface
                  , protected smsc::inman::interaction::PacketListenerIface {
protected:
  typedef smsc::inman::interaction::PckBuffersPool_T<1536> PacketsPool;

  typedef std::map<ConnectUId, ConnectGuard> ConnectsMap;
  typedef std::list<ConnectGuard> ConnectsList;
  typedef smsc::util::POBJRegistry_T<ConnectUId, USSConnManager> SessionsRegistry;

  mutable smsc::core::synchronization::EventMonitor _sync;
  /* - */
  const char *      _logId;    //logging prefix
  Logger *          _logger;

  ConnectsPool      _connPool; //pool of Connect objects
  ConnectsMap       _connMap;  //Connects monitored by this service.
  ConnectsList      _corpses;  //list of dead Connects which have no ConnManager assigned
  PacketsPool       _pckPool;  //incoming packet buffers pool
  SessionsRegistry  _sessReg;
  ConnectManagerID  _lastSessId;

  USSConnectCfg     _mgrCfg;

  smsc::inman::interaction::TcpServerCFG  _tcpCfg;
  smsc::inman::interaction::TcpServer     _tcpSrv;

  smsc::inman::inap::TCDsp_CFG            _ss7Cfg;
  smsc::inman::inap::TCAPDispatcher       _tcDisp;

  //Creates a connect manager serving given connect.
  //Returns true on success, false -  otherwise.
  bool setConnListener(const ConnectGuard & use_conn) /*throw()*/;
  //Handles USSConnManager destruction upon disconnection.
  void onDisconnect(const ConnectGuard & use_conn) /*throw()*/;

public:
  static const interaction::INPUSSGateway  _iProtoDef; //provided protocol definition

  explicit USSGService(const USSGateway_XCFG & in_cfg, const char * log_id = NULL,
              Logger * use_log = NULL)
    : _logId(log_id ? log_id : "USSMan")
    , _logger(use_log ? use_log : Logger::getInstance("smsc.ussman"))
    , _lastSessId(0), _tcpSrv(NULL, _logger)
  {
    _mgrCfg._denyDupRequest = in_cfg._denyDupRequest;
    _mgrCfg._maxRequests = in_cfg._maxRequests;
    _mgrCfg._maxThreads = in_cfg._maxThreads;
    _mgrCfg._wrkCfg._tcUsr = in_cfg._tcUsr;
    _tcpCfg = in_cfg._tcp;
    _ss7Cfg = in_cfg._ss7;
  }
  ~USSGService();


  bool init(void);

  bool start(void);

  void stop(bool do_wait = false);

protected:
  // -------------------------------------------
  // -- TcpServerListenerIface interface mthods
  // -------------------------------------------
  //Notifies that incoming connection with remote peer is accepted on given
  //socket.  If listener ignores connection, the 'use_sock' must be kept
  //intact and NULL must be returned.
  virtual SocketListenerIface *
    onConnectOpening(TcpServerIface & p_srv, ConnectUId conn_id, std::auto_ptr<Socket> & use_sock);
  //Notifies that connection is to be closed on given soket, no more events will be reported.
  virtual void onConnectClosing(TcpServerIface & p_srv, ConnectUId conn_id);
  //notifies that TcpServer is shutdowned, no more events on any connect will be reported.
  virtual void onServerShutdown(TcpServerIface & p_srv, TcpServerIface::RCode_e down_reason);

  // ------------------------------------------
  // -- PacketListenerIface interface mthods
  // ------------------------------------------
  //Returns true if listener has utilized packet so no more listeners
  //should be notified, false - otherwise (in that case packet will be
  //reported to other listeners).
  virtual bool onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
    /*throw(std::exception) */;

  //Returns true if listener has processed connect exception so no more
  //listeners should be notified, false - otherwise (in that case exception
  //will be reported to other listeners).
  virtual bool onConnectError(unsigned conn_id, 
                              PckAccumulatorIface::Status_e err_status,
                              const CustomException * p_exc = NULL)
    /*throw(std::exception) */;
};


} //ussman
} //smsc

#endif /* _SMSC_USS_GATEWAY_SERVICE_HPP */

