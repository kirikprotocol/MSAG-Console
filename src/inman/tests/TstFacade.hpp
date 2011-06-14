/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef _INMAN_TEST_FACADE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define _INMAN_TEST_FACADE_HPP

#include "core/synchronization/Event.hpp"

#include "inman/interaction/PckListenerDefs.hpp"
#include "inman/interaction/asynconn/Connect.hpp"
#include "inman/interaction/asynconn/PckBufferStore.hpp"
#include "inman/interaction/tcpserver/TcpServerDefs.hpp"
#include "inman/interaction/serializer/IMessages.hpp"

namespace smsc  {
namespace inman {
namespace test {

using smsc::logger::Logger;
using smsc::core::network::Socket;
using smsc::util::CustomException;

using smsc::core::synchronization::Mutex;

using smsc::inman::interaction::PacketBufferAC;
using smsc::inman::interaction::PckAccumulatorIface;
using smsc::inman::interaction::INPPacketIface;
using smsc::inman::interaction::SocketListenerIface;
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::TcpServerIface;

/* ************************************************************************** *
 * class TSTFacadeAC: 
 * ************************************************************************** */
#define MAX_FACADE_NAME 50
class TSTFacadeAC : public smsc::inman::interaction::TcpServerListenerIface
                  , public smsc::inman::interaction::PacketListenerIface {
protected:
  typedef smsc::inman::interaction::PckBuffersPool_T<2048> PacketsPool;

  mutable Mutex     _sync;
  PacketsPool       _pckPool;
  Connect           _pipe;
  TcpServerIface *  _tcpSrv;
  Logger *          logger;
    //logging prefix, f.ex: "TSTFacadeAC[%u]"
  char              _logId[MAX_FACADE_NAME + sizeof("[%u]") + sizeof(unsigned)*3 + 1];

  void do_disconnect(void);

public:
  explicit TSTFacadeAC(TcpServerIface & conn_srv, Logger * use_log = NULL)
    : _tcpSrv(&conn_srv), logger(use_log ? use_log : Logger::getInstance("smsc.InTST"))
  {
#ifdef __GRD_POOL_DEBUG__
    _pckPool.debugInit("InTST", logger);
#endif /* __GRD_POOL_DEBUG__ */
  }
  virtual ~TSTFacadeAC()
  {
    disconnect();
  }

  void Prompt(Logger::LogLevel dlvl, const char * zstr);
  void Prompt(Logger::LogLevel dlvl, const std::string & str)
  {
    Prompt(dlvl,str.c_str());
  }

  void disconnect(void);

  bool isActive(void) const;

  unsigned initConnect(const char* use_host, int use_port);

  //Customized variant of Connect::sendPck(): it sends specified
  //number of bytes from PacketBuffer
  //Returns zero on success, negative value in case of socket error,
  //positive value in case of invalid buffer data arrangement.
  int  sendPckPart(INPPacketIface * snd_pck, uint32_t num_bytes = 0);


  // ----------------------------------------------
  // -- TcpServerListenerIface interface methods:
  // ----------------------------------------------
  //Notifies that incoming connection with remote peer is accepted on given
  //socket.  If listener isn't interested in connection, the 'use_sock' must
  //be kept intact and NULL must be returned.
  virtual SocketListenerIface *
    onConnectOpening(TcpServerIface & p_srv, std::auto_ptr<Socket> & use_sock);
  //Notifies that connection is to be closed on given soket, no more events will be reported.
  virtual void onConnectClosing(TcpServerIface & p_srv, unsigned conn_id);
  //notifies that TcpServer is shutdowned, no more events on any connect will be reported.
  virtual void onServerShutdown(TcpServerIface & p_srv, TcpServerIface::RCode_e down_reason);

  // ------------------------------------------------------------
  // -- PacketListenerIface interface methods:
  // ------------------------------------------------------------
  //Returns true if listener has utilized packet so no more listeners
  //should be notified, false - otherwise (in that case packet will be
  //reported to other listeners).
  virtual bool onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
    /*throw(std::exception) */= 0;

  //Returns true if listener has processed connect exception so no more
  //listeners should be notified, false - otherwise (in that case exception
  //will be reported to other listeners).
  virtual bool onConnectError(unsigned conn_id,
                              PckAccumulatorIface::Status_e err_status,
                              const CustomException * p_exc = NULL)
    /*throw(std::exception) */;
};

} // namespace test
} // namespace inman
} // namespace smsc
#endif /* _INMAN_TEST_FACADE_HPP */

