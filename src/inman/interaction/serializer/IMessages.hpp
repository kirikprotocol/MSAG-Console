/* ************************************************************************* *
 * INMan Protocols: generic packet form and serialization definition.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INTERACTION_MESSAGES__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_MESSAGES__

#include "inman/interaction/IProtocol.hpp"
#include "inman/interaction/serializer/SerializableObj.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/*
 * INMan messages are transferred as length prefixed packet of two
 * serialized objects(header and command) and have the following
 * serialization format:

  2b        2b       up to ...b          up to ...b
-------  ---------   ----------   -------------------------------------
 msgId : hdrFormat : hdrData    :  message data                        |
                     |          |                                      |                                    |
                      ---------  -- processed by load()/save() method --
*/
class INPPacketIface : public SerializablePacketIface { //[0] = header, [1] = cmdObject
protected:
  using SerializablePacketIface::at;

  INPPacketIface()
  { }
  virtual ~INPPacketIface()
  { }

public:
  SerializableObjIface * pHdr(void) { return &at(0); }
  SerializableObjIface * pCmd(void) { return &at(1); }

  const SerializableObjIface * pHdr(void) const { return &at(0); }
  const SerializableObjIface * pCmd(void) const { return &at(1); }

  // ------------------------------------------------------
  // -- SerializablePacketIface interface implementation
  // ------------------------------------------------------
  //
  //virtual const SerializableObjIface & at(uint16_t use_idx) const = 0;
  //
  //virtual SerializableObjIface & at(uint16_t use_idx) = 0;
  //
  virtual uint16_t numObjects(void) const { return 2; }
  //
  virtual void serialize(PacketBufferAC & out_buf) const throw(SerializerException);
  //
  virtual DsrlzMode_e deserialize(PacketBufferAC & in_buf,
                                  DsrlzMode_e use_mode = dsmComplete) throw(SerializerException);
};

//Template class for solid packet construction.
template< class _Header /* : public SerializableObjIface */,
          class _Command /* : public SerializableObjIface */
>
class INPPacket_T : public INPPacketIface {
protected:
  // ------------------------------------------------------
  // -- SerializablePacketIface interface implementation
  // ------------------------------------------------------
  virtual const SerializableObjIface & at(uint16_t use_idx) const
  {
    if (use_idx > 0)
      return _Cmd;
    return _Hdr;
  }
  virtual SerializableObjIface & at(uint16_t use_idx)
  {
    if (use_idx > 0)
      return _Cmd;
    return _Hdr;
  }

public:
  _Header  _Hdr;
  _Command _Cmd;

  INPPacket_T() : INPPacketIface()
  { }
  //
  ~INPPacket_T()
  { }
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_MESSAGES__ */

