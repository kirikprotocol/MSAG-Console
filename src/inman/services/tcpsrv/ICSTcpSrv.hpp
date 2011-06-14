/* ************************************************************************* *
 * TCP Server service.
 * ************************************************************************* */
#ifndef __INMAN_ICS_TCPSERVER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_TCPSERVER_HPP

#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/tcpsrv/ICSTcpSrvDefs.hpp"
#include "inman/interaction/tcpserver/TcpServer.hpp"

namespace smsc {
namespace inman {
namespace tcpsrv {

using smsc::util::CustomException;
using smsc::core::network::Socket;

using smsc::inman::interaction::ConnectsPool;
using smsc::inman::interaction::TcpServerCFG;
using smsc::inman::interaction::PacketBufferAC;
using smsc::inman::interaction::PckAccumulatorIface;
using smsc::inman::interaction::SocketListenerIface;
using smsc::inman::interaction::TcpServerIface;
using smsc::inman::interaction::TcpServer;


class ICSTcpServer : public smsc::inman::ICServiceAC_T<TcpServerCFG>
                    , public ICSTcpServerIface
                    , smsc::inman::interaction::TcpServerListenerIface
                    , smsc::inman::interaction::PacketListenerIface {
public:
  ICSTcpServer(std::auto_ptr<TcpServerCFG> & use_cfg,
               const ICServicesHostITF * svc_host, Logger * use_log = NULL)
      : smsc::inman::ICServiceAC_T<TcpServerCFG>(ICSIdent::icsIdTCPServer,
                                                  svc_host, use_cfg, use_log)
      , _tcpSrv("TCPSrv", use_log)
  {
    _cfg.reset(use_cfg.release());
    _icsState = ICServiceAC::icsStConfig;
    _logId = _tcpSrv.getIdent();
    if (!logger)
      logger = Logger::getInstance("smsc.inman");
#ifdef __GRD_POOL_DEBUG__
    _pckPool.debugInit(_logId, logger);
#endif /* __GRD_POOL_DEBUG__ */
  }
  virtual ~ICSTcpServer();

  // ---------------------------------
  // -- ICServiceAC interface methods
  // --------------------------------- 
  //Returns ICSTcpServerIface
  void * Interface(void) const { return (ICSTcpServerIface*)this; }

  // ------------------------------------------
  // -- ICSTcpServerIface interface methods
  // ------------------------------------------
  //Registers an ICService providing specified protocol functionality.
  //Returns false if given protocol is already provided by registered ICService.
  virtual bool registerProtocol(ICSConnServiceIface & conn_srv);
  //Unregisters an ICService providing specified protocol functionality.
  virtual bool unregisterProtocol(const IProtocolId_t & proto_id);

private:
  typedef std::list<ConnectGuard> ConnectsList;

  struct ConnectInfo {
    ConnectGuard    _grd;
    IProtocolId_t   _protoId;
  };
  typedef std::map<unsigned /*sock_id*/, ConnectInfo> ConnectsMap;

  struct ProtocolInfo {
    ICSConnServiceIface * _srv; //sevice provider
    uint16_t              _refs;  //counter of refs TcpServer set to service provoder

    explicit ProtocolInfo(ICSConnServiceIface * use_srv = NULL)
      : _srv(use_srv), _refs(0)
    { }
  };
  typedef std::map<IProtocolId_t, ProtocolInfo> ProtocolsMap;
  typedef smsc::inman::interaction::PckBuffersPool_T<2048> PacketsPool;
  
  /* - */
  mutable Mutex   _sync;
  ConnectsList    _corpses;  //list of dead Connects, which have no protocol assigned
  ConnectsPool    _connPool; //pool of Connect objects
  PacketsPool     _pckPool;  //pool of buffers used for acquiring of 1st
                             //packet on accepted connections.
  ProtocolsMap    _protoReg; //registered protocols and services
  ConnectsMap     _connMap;  //Connects monitored by this server.
  const char *    _logId;    //logging prefix
  /* - */
  std::auto_ptr<TcpServerCFG>         _cfg;
  TcpServer       _tcpSrv;

  //Attempts to detect protocol the received packet belongs to.
  //Return false in case of failure.
  bool detectProto(const PacketBufferAC & recv_pck, ProtocolsMap::iterator & out_cit);

protected:
  // ---------------------------------
  // -- ICServiceAC interface methods
  // --------------------------------- 
  virtual Mutex & _icsSync(void) const { return _sync; }
  //Initializes service verifying that all dependent services are inited
  virtual RCode _icsInit(void);
  //Starts service verifying that all dependent services are started
  virtual RCode _icsStart(void);
  //Stops service
  virtual void  _icsStop(bool do_wait = false);

  // -------------------------------------------
  // -- TcpServerListenerIface interface mthods
  // -------------------------------------------
  //Notifies that incoming connection with remote peer is accepted on given
  //socket.  If listener isn't interested in connection, the 'use_sock' must
  //be kept intact and NULL must be returned.
  virtual SocketListenerIface *
    onConnectOpening(TcpServerIface & p_srv, std::auto_ptr<Socket> & use_sock);
  //Notifies that connection is to be closed on given soket, no more events will be reported.
  virtual void onConnectClosing(TcpServerIface & p_srv, unsigned conn_id);
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

} //tcpsrv
} //inman
} //smsc
#endif /* __INMAN_ICS_TCPSERVER_HPP */

