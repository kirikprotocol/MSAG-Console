/* ************************************************************************** *
 * Packet Accumulator: structure that accumulates data of generic packet.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_ASYNCONN_PCK_ACCUM
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_ASYNCONN_PCK_ACCUM

#include "util/Exception.hpp"
#include "util/OptionalObjT.hpp"
#include "inman/interaction/PacketBuffer.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

class PckAccumulatorIface {
public:
  enum Status_e { //packet data accumulation status
    accSockErr = -3     //failed due to fatal socket error
    , accDataErr = -2   //failed due to invalid data arrangement
    , accEOF = -1       //failed because of socket is closed by one of peers.
    , accAwaits = 0     //more data is awaited
    , accComplete = 1   //succeeded
  };

  unsigned    _connId; //connection/socket id
  Status_e    _accStatus;
  /* */
  smsc::util::OptionalObj_T<smsc::util::CustomException>  _exc;


  static const char * nmStatus(Status_e use_st)
  {
    switch (use_st) {
    case PckAccumulatorIface::accSockErr:   return "accSockErr";
    case PckAccumulatorIface::accDataErr:   return "accDataErr";
    case PckAccumulatorIface::accEOF:       return "accEOF";
    case PckAccumulatorIface::accAwaits:    return "accAwaits";
    case PckAccumulatorIface::accComplete:  return "accComplete";
    default:;
    }
    return "unknown";
  }

  bool isComplete(void) const { return (_accStatus == accComplete); }

  void reset(void)
  {
    _accStatus = accAwaits;
    _exc.clear();
    pckBuf().reset(); 
  }

  // ---------------------------------------
  // -- PckAccumulatorIface
  // ---------------------------------------
  virtual PacketBufferAC::size_type maxSize(void) const = 0;
  virtual PacketBufferAC & pckBuf(void) = 0;

protected:
  PckAccumulatorIface() : _connId(0), _accStatus(accAwaits)
  { }
  virtual ~PckAccumulatorIface()
  { }
};

template <uint32_t MAX_PACKET_SZ>
class PckAccumulator_T : public PckAccumulatorIface {
protected:
  PacketBuffer_T<MAX_PACKET_SZ> _dBuf;
  
public:
  PckAccumulator_T() : PckAccumulatorIface()
  { }
  virtual ~PckAccumulator_T()
  { }

  // ----------------------------------------------------
  // -- PckAccumulatorIface interface methods
  // ----------------------------------------------------
  virtual PacketBufferAC::size_type maxSize(void) const { return MAX_PACKET_SZ; }
  virtual PacketBufferAC & pckBuf(void) { return _dBuf; }
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_ASYNCONN_PCK_ACCUM */

