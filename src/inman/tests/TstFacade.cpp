#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/tests/TstFacade.hpp"

using smsc::core::network::Socket;
using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::INPSerializer;

namespace smsc  {
namespace inman {
namespace test {
/* ************************************************************************** *
 * class TSTFacadeAC implementation:
 * ************************************************************************** */
void TSTFacadeAC::Prompt(Logger::LogLevel dlvl, const char * zstr)
{
  fprintf(stdout, "%s\n", zstr);
  logger->log(dlvl, "%s: %s", _logId, zstr);
}

void TSTFacadeAC::do_disconnect(void)
{
  if (_pipe) {
    _connSrv->rlseConnection(_pipe->getId());
    delete _pipe; _pipe = NULL;
  }
}

void TSTFacadeAC::Disconnect(void)
{
  MutexGuard grd(_sync);
  do_disconnect();
}

bool TSTFacadeAC::isActive(void) const
{
  MutexGuard grd(_sync);
  return is_active();
}

unsigned TSTFacadeAC::initConnect(const char* use_host, int use_port)
{
  MutexGuard grd(_sync);
  do_disconnect();
  {
    std::string msg;
    format(msg, "%s: connecting to InManager at %s:%d...\n", _logId, use_host, use_port);
    smsc_log_info(logger, msg.c_str());
    fprintf(stdout, "%s", msg.c_str());
  }
  Socket * socket = _connSrv->setConnection(use_host, use_port);
  if (socket) {
    _pipe = new Connect(socket, INPSerializer::getInstance(), logger);
    _pipe->addListener(this);
    _connSrv->addConnection(_pipe, this);
    {
      std::string msg;
      format(msg, "%s: connect[%u] created\n", _logId, _pipe->getId());
      smsc_log_info(logger, msg.c_str());
      fprintf(stdout, "%s", msg.c_str());
    }
    return _pipe->getId();
  }
  return 0;
}

//Customized variant of Connect::sendPck(): it sends specified
//number of bytes from ObjectBuffer
int  TSTFacadeAC::sendPckPart(INPPacketAC *pck, uint32_t num_bytes/* = 0*/)
{
  ObjectBuffer  buffer(1280);

  buffer.setPos(4);
  pck->serialize(buffer);
  num_bytes = !num_bytes ? (uint32_t)buffer.getPos() - 4 : num_bytes;
  {
    uint32_t len = htonl(num_bytes);
    memcpy(buffer.get(), (const void *)&len, 4);
  }

  MutexGuard grd(_sync);
  if (is_active())
    return _pipe->send(buffer.get(), num_bytes + 4);

  Prompt(Logger::LEVEL_ERROR, "ERR: IN manager doesn't connected!");
  return 0;
}

// ---------------------------------------------------
// -- ConnectSupervisorITF interface implementation
// ---------------------------------------------------
bool TSTFacadeAC::onConnectClosed(ConnectAC * conn)
{
  MutexGuard grd(_sync);
  _pipe = NULL;
  return true;  //ConnSrv will destroy connect
}

// ---------------------------------------------------
// -- ConnectListenerITF interface implementation
// ---------------------------------------------------

void TSTFacadeAC::onConnectError(Connect* conn, std::auto_ptr<CustomException> & p_exc)
{
  MutexGuard grd(_sync);
  Prompt(Logger::LEVEL_ERROR, p_exc->what());
  _pipe = NULL; //ConnSrv will destroy connect
}

} // test
} // namespace inman
} // namespace smsc

