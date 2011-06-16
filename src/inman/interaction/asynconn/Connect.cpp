#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "util/BinDump.hpp"
using smsc::util::DumpHex;
//using smsc::util::DumpDbg;

#include "inman/interaction/asynconn/Connect.hpp"
using smsc::core::synchronization::MutexGuard;

namespace smsc  {
namespace inman {
namespace interaction {

/* ************************************************************************** *
 * class Connect implementation:
 * ************************************************************************** */
Connect::Connect(std::auto_ptr<Socket> & use_sock, Logger * use_log/* = NULL*/)
  : SocketListenerIface(use_sock), _pckPool(NULL)
  , _logger(use_log ? use_log : Logger::getInstance(ASYNCONN_DFLT_LOGGER))
{
  snprintf(_logId, sizeof(_logId)-1, "Conn[%u]", getId());
  _pckNtfr.init(_logId, _logger);
}

//
Connect::~Connect()
{
  stop();
  if (isOpened())
    getSocket()->Abort();
  if (_logger) {
    smsc_log_debug(_logger, "%s: destroyed", _logId);
  }
}

//Binds asynchronous Connect to given socket, initializes ident string.
void Connect::bind(std::auto_ptr<Socket> & use_sock, Logger * use_log/* = NULL*/)
{
  MutexGuard  grd(_rcvSync);
  SocketListenerIface::assignSocket(use_sock);
  _logger = use_log ? use_log : Logger::getInstance(ASYNCONN_DFLT_LOGGER);
  snprintf(_logId, sizeof(_logId)-1, "Conn[%u]", getId());
  _pckNtfr.init(_logId, _logger);
}

//Returns false if socket is not opened or no PacketBuffer is available in given pool.
bool Connect::init(PckBuffersPoolAC & pck_pool, uint16_t max_threads/* = 0*/)
{
  _pckNtfr.setThreads(1, max_threads);

  MutexGuard  grd(_rcvSync);
  _pckPool = &pck_pool;
  return ((!_pckGrd.get() && prepareNextBuffer()) && isOpened());
}
//Resets incoming packet accumulation, already acquired data is lost.
//Returns false if socket is not opened or no PacketBuffer is available.
bool Connect::reset(void)
{
  MutexGuard  grd(_rcvSync);
  return (prepareNextBuffer() && isOpened());
}

//Sends data stored in PacketBuffer (starting from its current position)
//to socket in length prefixed format.
//Returns zero on success, negative value in case of socket error,
//positive value in case of invalid PacketBuffer arrangement. 
int  Connect::sendPck(const PacketBufferAC & use_pck) /*throw()*/
{
  int32_t dataSz = use_pck.getDataSize() - use_pck.getPos();
  if (dataSz < 0)
    return 1;

  union { //force correct alignment of len.buf on 64-bit platforms
    uint32_t    ui;
    uint8_t     buf[sizeof(uint32_t)];
  } len;

  len.ui = htonl((uint32_t)dataSz);

  MutexGuard  tmp(_sndSync);
  int n = send(len.buf, 4);
  if (n > 0)
    n = send(use_pck.getCurPtr(), (uint32_t)dataSz);
  return (n < 0) ? n : 0;
}

bool Connect::hasExceptionOnSend(smsc::util::CustomException * p_exc/* = NULL*/) const
{
  MutexGuard  tmp(_sndSync);
  if (_sndExc.get()) {
    if (p_exc)
      *p_exc = *_sndExc.get();
    return true;
  }
  return false;
}

/* -------------------------------------------------------------------------- *
 * SocketListenerIface interface implementation:
 * -------------------------------------------------------------------------- */
ConnectState_e Connect::onReadEvent(void)  /*throw()*/
{
  MutexGuard grd(_rcvSync);

  switch (_pckAcq.readSocket(_socket.get())) {
  case PckAccumulatorIface::accSockErr: {
    smsc_log_error(_logger, "%s", _pckGrd->_exc->what());
    switchState(SocketListenerIface::connException);
    _pckAcq.release();
    _pckNtfr.onPacketEvent(_pckGrd);
    _pckGrd.release();
  } break;

  case PckAccumulatorIface::accDataErr: {
    smsc_log_error(_logger, "%s: packets arrangement error", _logId);
    switchState(SocketListenerIface::connException);
    _pckAcq.release();
    _pckNtfr.onPacketEvent(_pckGrd);
    _pckGrd.release();
  } break;

  case PckAccumulatorIface::accEOF: {
    smsc_log_info(_logger, "%s: remote peer closed socket", _logId);
    switchState(SocketListenerIface::connEOF);
    _pckAcq.release();
    _pckNtfr.onPacketEvent(_pckGrd);
    _pckGrd.release();
  } break;

  case PckAccumulatorIface::accComplete: {
    _pckAcq.release();
    _pckNtfr.onPacketEvent(_pckGrd);
    _pckGrd.release();
    prepareNextBuffer();
  } break;
  
  // case PckAccumulatorIface::acqAwaits:
  default:;
  } /* eosw */
  return getState();
}

//passes Connect exception to connect listeners
void Connect::onCloseEvent(int err_no) /*throw()*/
{
  MutexGuard grd(_rcvSync);
  switchState((_pckAcq.onDisconnect(err_no) == PckAccumulatorIface::accEOF)
                ? SocketListenerIface::connEOF : SocketListenerIface::connException);
  _pckAcq.release();
  _pckNtfr.onPacketEvent(_pckGrd);
  _pckGrd.release();
}

/* -------------------------------------------------------------------------- *
 * Private/Protected methods:
 * -------------------------------------------------------------------------- */
//sends given number of bytes to socket
//returns number of bytes sent, or -1 on error and sets _sndExc
int  Connect::send(const uint8_t * use_buf, int32_t buf_sz)
{
  if (_sndExc.get())
    return -1;

  int n = _socket->Write((const char *)use_buf, buf_sz);
  if ((n < buf_sz) || _logger->isDebugEnabled()) {
    std::string         dstr;
    Logger::LogLevel    errLvl;

    smsc::util::format(dstr, "%s: sent %d of %d bytes", _logId, n, buf_sz);
    if (n > 0) {
      dstr += ": 0x"; DumpHex(dstr, n, use_buf);
    }
    if (n < buf_sz) {
      errLvl = Logger::LEVEL_ERROR;
      _sndExc.init().fill(dstr.c_str(), errno, strerror(errno));
    } else
      errLvl = Logger::LEVEL_DEBUG;

    _logger->log(errLvl, dstr.c_str());
  }
  return (n < buf_sz) ? (-1) : n;
}

//Returns false if no more PacketBuffer available
bool Connect::prepareNextBuffer(void)
{
  _pckGrd = _pckPool->allcObj();
  if (_pckGrd.get()) {
    _pckGrd->reset();
    _pckGrd->_connId = SocketListenerIface::getId();
    _pckAcq.init(*_pckGrd, _logId, _logger);
    return true;
  }
  _pckAcq.release();
  return false;
}


} // namespace interaction
} // namespace inman
} // namespace smsc

