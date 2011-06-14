#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "util/BinDump.hpp"
using smsc::util::DumpHex;
//using smsc::util::DumpDbg;

#include "inman/interaction/asynconn/PckAcquirer.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

/* ************************************************************************** *
 * classes PckAcquirer implementation:
 * ************************************************************************** */
void PckAcquirer::init(PckAccumulatorIface & use_acc, const char * log_id, Logger * use_log)
{
  _num2Read = 4;
  _numRcvd = 0;
  _state = pckIdle;
  _logId = log_id;
  _logger = use_log;
  _pAcc = &use_acc;
  _pAcc->reset();
}

void PckAcquirer::release(void)
{
  _num2Read = 4;
  _numRcvd = 0;
  _state = pckIdle;
  _pAcc = NULL;
}

//Read next portion of data from socket
PckAccumulatorIface::Status_e PckAcquirer::readSocket(Socket * use_sock)  /*throw()*/
{
  int n = use_sock->Read((char*)curDataBuf() + _numRcvd, _num2Read - _numRcvd);
  if (!n)
    return _pAcc->_accStatus = PckAccumulatorIface::accEOF;
  if (n < 0)
    return  _pAcc->_accStatus = ((errno != EINTR) ?
                         onSocketError(errno) : PckAccumulatorIface::accAwaits);
  _numRcvd += n;
  return _pAcc->_accStatus = ((_numRcvd == _num2Read) ?
                              onPartReceived() : PckAccumulatorIface::accAwaits);
}
//
PckAccumulatorIface::Status_e PckAcquirer::onDisconnect(int err_no/* = 0*/) /*throw()*/
{
  if (err_no)
    return _pAcc->_accStatus = onSocketError(err_no);

  if ((_pAcc->_accStatus == PckAccumulatorIface::accAwaits)
      && (_numRcvd || (_state != PckAcquirer::pckIdle)))
    _pAcc->_exc.init().fill(err_no, "%s: socket unexpectedly closed", _logId);
  return _pAcc->_accStatus = PckAccumulatorIface::accEOF;
}

// ------------------------------------------------------------
// Protected methods
// ------------------------------------------------------------
PckAccumulatorIface::Status_e PckAcquirer::onPartReceived(void) /*throw()*/
{
  smsc_log_debug(_logger, "%s: received %u of %ub: %s%s", _logId, _numRcvd, _num2Read,
                 "0x", DumpHex(_numRcvd, curDataBuf()).c_str());
//                "\n", DumpDbg(_numRcvd, curDataBuf()).c_str());
  if (_state == pckIdle) {  //packet length has just been read
    uint32_t oct2read = ntohl(*(uint32_t*)_len.buf);
    _len.ui = oct2read;
    if (oct2read > _pAcc->maxSize()) {
      _pAcc->_exc.init().fill(-1, "%s: incoming packet is too large: %ub", _logId, oct2read);
      return PckAccumulatorIface::accDataErr;
    }
    _pAcc->pckBuf().reset(_num2Read = oct2read);
    _numRcvd = 0;
    _state = pckLength;
    return PckAccumulatorIface::accAwaits;
  }
  //rest of packet was read, finilize objBuf, (_state == pckLength)
  _pAcc->pckBuf().setDataSize(_len.ui);
  _state = pckBody;
  return PckAccumulatorIface::accComplete;
}

PckAccumulatorIface::Status_e PckAcquirer::onSocketError(int err_no) /*throw()*/
{
  _pAcc->_exc.init().fill(err_no, "%s: socket error, code %d (%s)", _logId,
                          err_no, err_no ? strerror(err_no) : "");
  return PckAccumulatorIface::accSockErr;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

