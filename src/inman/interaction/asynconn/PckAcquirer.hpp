/* ************************************************************************** *
 * PckAcquirer: accumulates data from socket until whole length prefixed
 * packet is read.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_ASYNCONN_PCK_ACQUIRER_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_ASYNCONN_PCK_ACQUIRER_DEFS

#include <errno.h>

#include "logger/Logger.h"
#include "core/network/Socket.hpp"

#include "inman/interaction/PckAccumulator.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

using smsc::core::network::Socket;
using smsc::logger::Logger;

class PckAcquirer {
private:
  enum PckState_e { pckIdle = 0, pckLength, pckBody };

  union { //force correct alignment of len.buf on 64-bit platforms
    uint32_t    ui;
    uint8_t     buf[sizeof(uint32_t)];
  } _len;

  PckState_e    _state;
  uint32_t      _numRcvd;
  uint32_t      _num2Read;
  const char *  _logId;
  Logger *      _logger;
  /* - */
  PckAccumulatorIface * _pAcc;

protected:
  //returns current data acquisition buffer
  uint8_t * curDataBuf(void)
  {
    return (_state < pckLength) ? _len.buf : _pAcc->pckBuf().get();
  }

  /* - */
  PckAccumulatorIface::Status_e onPartReceived(void) /*throw()*/;
  PckAccumulatorIface::Status_e onSocketError(int err_no = 0) /*throw()*/;

public:
  PckAcquirer() : _state(pckIdle), _numRcvd(0), _num2Read(4)
    , _logId(NULL), _logger(NULL), _pAcc(NULL)
  {
    _len.ui = 0;
  }
  //
  ~PckAcquirer()
  { }

  void init(PckAccumulatorIface & use_acc, const char * log_id, Logger * use_log);

  void release(void);

  //Read next portion of data from socket
  PckAccumulatorIface::Status_e readSocket(Socket * use_sock) /*throw()*/;
  //Finalize PckAcquirer state on connection closing/abort.
  PckAccumulatorIface::Status_e onDisconnect(int err_no = 0) /*throw()*/;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_ASYNCONN_PCK_ACQUIRER_DEFS */

