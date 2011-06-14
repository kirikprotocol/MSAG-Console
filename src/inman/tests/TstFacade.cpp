#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/tests/TstFacade.hpp"
using smsc::util::format;
using smsc::core::synchronization::MutexGuard;

namespace smsc  {
namespace inman {
namespace test {

/* ************************************************************************** *
 * class TSTFacadeAC implementation:
 * ************************************************************************** */
void TSTFacadeAC::Prompt(Logger::LogLevel dlvl, const char * zstr)
{
  //Matches the enum Logger::LogLevel .
  static const char * const _prmtPrefix[] = {
    "", "DBG: ", "INF: ", "WRN: ", "ERR: ", "FTL: "
  };

  fprintf(stdout, "%s%s\n", _prmtPrefix[dlvl], zstr);
  logger->log(dlvl, "%s: %s", _logId, zstr);
}

void TSTFacadeAC::do_disconnect(void)
{
  if (_pipe.isOpened())
    _tcpSrv->rlseConnectionWait(_pipe.getId());
  _pipe.stop();
  _pipe.clearListeners();
}

void TSTFacadeAC::disconnect(void)
{
  MutexGuard grd(_sync);
  do_disconnect();
}

bool TSTFacadeAC::isActive(void) const
{
  MutexGuard grd(_sync);
  return _pipe.isOpened();
}

unsigned TSTFacadeAC::initConnect(const char* use_host, int use_port)
{
  MutexGuard grd(_sync);
  do_disconnect();
  {
    std::string msg;
    format(msg, "connecting to InManager at %s:%d...\n", use_host, use_port);
    Prompt(Logger::LEVEL_INFO, msg.c_str());
  }
  std::auto_ptr<Socket> pSock;
  if (_tcpSrv->setConnection(use_host, use_port, pSock)) {
    _pipe.bind(pSock, logger);
    _pipe.init(_pckPool, 1); //set consequitive mode
    _pipe.addListener(*this);
    _pipe.start();
    _tcpSrv->registerConnection(_pipe, true);
    {
      std::string msg;
      format(msg, "Connect[%u] created\n", _pipe.getId());
      Prompt(Logger::LEVEL_INFO, msg.c_str());
    }
    return _pipe.getId();
  }
  return 0;
}

//Customized variant of Connect::sendPck(): it sends specified
//number of bytes from PacketBuffer
//Returns zero on success, negative value in case of socket error,
//positive value in case of invalid buffer data arrangement.
int TSTFacadeAC::sendPckPart(INPPacketIface * snd_pck, uint32_t num_bytes/* = 0*/)
{
  smsc::inman::interaction::PacketBuffer_T<1280>  pckBuf;

  try {
    snd_pck->serialize(pckBuf);
    pckBuf.setPos(0);
  } catch (const std::exception & exc) {
    std::string msg;
    format(msg, "serialization exception: %s\n", exc.what());
    Prompt(Logger::LEVEL_FATAL, msg.c_str());
    return -1;
  }

  if (num_bytes && (num_bytes < pckBuf.getDataSize()))
    pckBuf.setDataSize(num_bytes);
   
  MutexGuard grd(_sync);
  if (_pipe.isOpened())
    return _pipe.sendPck(pckBuf);

  Prompt(Logger::LEVEL_ERROR, "ERR: IN manager doesn't connected!");
  return -1;
}

// ----------------------------------------------
// -- TcpServerListenerIface interface methods:
// ----------------------------------------------
//Notifies that incoming connection with remote peer is accepted on given
//socket.  If listener isn't interested in connection, the 'use_sock' must
//be kept intact and NULL must be returned.
SocketListenerIface *
  TSTFacadeAC::onConnectOpening(TcpServerIface & p_srv, std::auto_ptr<Socket> & use_sock)
{
  MutexGuard grd(_sync);
  Prompt(Logger::LEVEL_WARN, "denying incoming connection request");
  return NULL;
}

//Notifies that connection is to be closed on given soket, no more events will be reported.
void TSTFacadeAC::onConnectClosing(TcpServerIface & p_srv, unsigned conn_id)
{
  MutexGuard grd(_sync);
  std::string msg;
  format(msg, "%s closed Connect[%u]", p_srv.getIdent(), conn_id);
  Prompt(Logger::LEVEL_INFO, msg.c_str());
  if (conn_id == (unsigned)_pipe.getId()) {
    _pipe.stop();
    _pipe.clearListeners();
  }
  return;
}

//notifies that TcpServer is shutdowned, no more events on any connect will be reported.
void TSTFacadeAC::onServerShutdown(TcpServerIface & p_srv, TcpServerIface::RCode_e down_reason)
{
  MutexGuard grd(_sync);
  std::string msg;
  format(msg, "%s is shutdowned, reason %u", p_srv.getIdent(), (unsigned)down_reason);
  Prompt(Logger::LEVEL_INFO, msg.c_str());
  _pipe.stop();
  _pipe.clearListeners();
}

// ------------------------------------------------------------
// -- PacketListenerIface interface methods:
// ------------------------------------------------------------

//Returns true if listener has processed connect exception so no more
//listeners should be notified, false - otherwise (in that case exception
//will be reported to other listeners).
bool TSTFacadeAC::onConnectError(unsigned conn_id,
                                 PckAccumulatorIface::Status_e err_status,
                                 const CustomException * p_exc/* = NULL*/)
  /*throw(std::exception) */
{
  if ((err_status != PckAccumulatorIface::accEOF) || p_exc) {
    std::string msg;
    format(msg, "Connect[%u] error status(%u): %s", conn_id,
                   (unsigned)err_status, p_exc ? p_exc->what() : "");
    Prompt(Logger::LEVEL_ERROR, msg.c_str());
  }
  _pipe.clearListeners();
  return true;
}

} // test
} // namespace inman
} // namespace smsc

