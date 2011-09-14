/* ************************************************************************** *
 * Multithreadsafe accumulative pool of packet buffers. 
 * Buffers access/releasing is guarded by reference counter. 
 * ************************************************************************** */
#ifndef __SMSC_INMAN_ASYNCONN_PCK_BUFFER_STORE
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_ASYNCONN_PCK_BUFFER_STORE

#include "core/buffers/IntrusivePoolOfIfaceT.hpp"
#include "inman/interaction/PckAccumulator.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

typedef smsc::core::buffers::IntrusivePoolAC_T<PckAccumulatorIface, uint32_t> PckBuffersPoolAC;
typedef PckBuffersPoolAC::ObjRef PckBufferGuard;

class PckBufferRefereeIface {
protected:
  virtual ~PckBufferRefereeIface()
  { }

public:
  //Returns false if event cann't be processed by referee
  virtual bool onPacketEvent(const PckBufferGuard & use_pck) = 0;
};

template <uint32_t _PACKET_SZ>
class PckBuffersPool_T : public 
  smsc::core::buffers::IntrusivePoolOfIfaceImpl_T<PckAccumulatorIface, PckAccumulator_T<_PACKET_SZ>, uint32_t, false> {
public:
  PckBuffersPool_T() //PacketBuffers are reusable
    : smsc::core::buffers::IntrusivePoolOfIfaceImpl_T<PckAccumulatorIface, PckAccumulator_T<_PACKET_SZ>, uint32_t, false>()
  { }
  ~PckBuffersPool_T()
  { }
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_ASYNCONN_PCK_BUFFER_STORE */

