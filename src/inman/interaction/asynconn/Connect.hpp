/* ************************************************************************** *
 * Asynchronous Connect: sends/receives length prefixed packets. 
 * Received packets asynchronously reported to connect listeners by pool
 * of threads. 
 * Note: the consecutive processing of received packets may be forced by
 *       setting max_threads = 1.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_ASYNC_CONNECT_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_ASYNC_CONNECT_HPP

#include "inman/interaction/SocketListenerDefs.hpp"
#include "inman/interaction/asynconn/PckAcquirer.hpp"
#include "inman/interaction/asynconn/PckNotifier.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

/* ************************************************************************** *
 * class Connect:
 * ************************************************************************** */
class Connect : public SocketListenerIface {
private:
  mutable Mutex       _sndSync;
  mutable Mutex       _rcvSync; //guards: _pckAcq, _pckGrd

  PckAcquirer         _pckAcq;
  PckBuffersPoolAC *  _pckPool;
  PckBufferGuard      _pckGrd;  //current packet acquisition buffer
  PckNotifier         _pckNtfr;
  char                _logId[sizeof("Conn[%u]") + sizeof(unsigned)*3 + 1];
  Logger *            _logger;
  /* */
  smsc::util::OptionalObj_T<smsc::util::CustomException>
                      _sndExc; //last sending exception, guarded by _sndSync

protected:
  void switchState(ConnectState_e new_st) { _connState = new_st; }
  //sends given number of bytes to socket
  //returns number of bytes sent, or -1 on error and sets _sndExc
  int send(const uint8_t * buf, int32_t bufSz) /*throw()*/;
  //Returns false if no more PacketBuffer available
  bool prepareNextBuffer(void);

public:
  Connect() : SocketListenerIface(), _pckPool(NULL), _logger(NULL)
  {
    _logId[0] = 0; //inited while bind() call
  }
  explicit Connect(std::auto_ptr<Socket> & use_sock, Logger * use_log = NULL);
  //
  virtual ~Connect()
  {
    stop();
    if (isOpened())
      getSocket()->Abort();
  }

  //Returns threads number limitation.
  //Note: Zero means no limit is set.
  uint16_t getMaxThreads(void) const { return (uint16_t)_pckNtfr.getMaxThreads(); }

  //Binds asynchronous Connect to given socket, initializes ident string.
  void bind(std::auto_ptr<Socket> & use_sock, Logger * use_log = NULL);
  //Initializes incoming packet accumulation, already acquired data is preserved.
  //Returns false if socket is not opened or no PacketBuffer is available in given pool.
  //Note.1: setting (max_threads == 1) forces the consecutive packets processing.
  //Note.2: it's safe to change PacketBuffers pool on already active Connect.
  bool init(PckBuffersPoolAC & pck_pool, uint16_t max_threads = 0);
  //Resets incoming packet accumulation, already acquired data is lost.
  //Returns false if socket is not opened or no PacketBuffer is available.
  bool reset(void);
  //
  void setMaxThreads(uint16_t max_threads) { _pckNtfr.setThreads(1, max_threads); }

  //Returns true if Connect is handling incoming packets.
  bool isRunning(void) const { return _pckNtfr.isRunning(); }

  //Starts notification of packet events to listeners.
  bool start(void) { return _pckNtfr.start(); }
  
  //Finally stops incoming packets acquisition.
  //Cancels processing of already accumulated packets.
  //If shutdown timeout isn't set the default one is used.
  void stop(const TimeSlice * use_tmo = NULL)
  {
    if (_pckNtfr.isRunning())
      _pckNtfr.stop(use_tmo);
  }

  // -------------------------------------------------------------------------------
  // NOTE: Connect object MUST NOT be destroyed from PacketListenerIface methods !!!
  // -------------------------------------------------------------------------------
  //
  void addListener(PacketListenerIface & use_lstr)
  {
    _pckNtfr.addListener(use_lstr);
  }
  //
  void removeListener(PacketListenerIface & use_lstr)
  {
    _pckNtfr.removeListener(use_lstr);
  }
  //
  void clearListeners(void) { _pckNtfr.clearListeners(); }

  //Sends packet to socket in length prefixed format,
  //returns zero on success, negative value in case of socket error,
  //positive value in case of invalid buffer data arrangement.
  int sendPck(const PacketBufferAC & use_pck) /*throw()*/;

  //returns last connect exception occured while sending data
  bool hasExceptionOnSend(smsc::util::CustomException * p_exc = NULL) const;
  //
  const char * logId(void) const { return &_logId[0]; }

  // ---------------------------------------------------
  // -- SocketListenerIface interface methods
  // ---------------------------------------------------
  //Notifies that either data is available for reading or normal EOF condition
  //is reached on socket.
  //Further connection state depends on returned status.
  virtual ConnectionState_e onReadEvent(void) /*throw()*/;
  //Notifies that socket is to be closed due to reason identified by 'err_no'
  //argument. Zero value means socket is closed by some external request.
  virtual void onCloseEvent(int err_no) /*throw()*/;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_ASYNC_CONNECT_HPP */

