/* ************************************************************************** *
 * Definition of generic listener of connection incoming packets.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_PACKET_LISTENER_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_PACKET_LISTENER_DEFS

#include "inman/interaction/PckAccumulator.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

class PacketListenerIface {
protected:
  virtual ~PacketListenerIface() //forbid interface destruction
  { }

public:
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
                              const smsc::util::CustomException * p_exc = NULL)
    /*throw(std::exception) */= 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_PACKET_LISTENER_DEFS */

