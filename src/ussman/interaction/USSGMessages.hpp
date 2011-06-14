/* ************************************************************************** *
 * USS Gateway protocol messages serialization format definition.
 * ************************************************************************** */
#ifndef __SMSC_USS_GATEWAY_MESSAGES_FMT
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USS_GATEWAY_MESSAGES_FMT

#include "inman/interaction/serializer/SerializableObj.hpp"

namespace smsc  {
namespace ussman {
namespace interaction {

using smsc::inman::interaction::PacketBufferAC;
using smsc::inman::interaction::SerializerException;
using smsc::inman::interaction::SerializableObjIface;

/*
 * USSMan messages are transferred as length prefixed packet
 * consisting of single serialized object(command) and have
 * the following serialization format:

  2b        4b              up to ...b
-------  ---------   -------------------------------
 msgId : requestId  :  message data                |
       |                                           |
           -- processed by load()/save() method --
*/
class USSGPacketAC : public smsc::inman::interaction::SerializablePacketIface { //[0] = cmdObject
protected:
  uint32_t    dlgId;

  explicit USSGPacketAC(uint32_t dlg_id = 0)
    : SerializablePacketIface(), dlgId(dlg_id)
  { }

public:
  virtual ~USSGPacketAC()
  { }
 
  SerializableObjIface * pCmd(void) { return &at(0); }
  const SerializableObjIface * pCmd(void) const { return &at(0); }

  uint32_t  getDlgId(void) const { return dlgId; }
  void      setDlgId(uint32_t dlg_id) { dlgId = dlg_id; }

  // ------------------------------------------------------
  // -- SerializablePacketIface interface implementation
  // ------------------------------------------------------
  //
  virtual const SerializableObjIface & at(uint16_t use_idx) const = 0;
  //
  virtual SerializableObjIface & at(uint16_t use_idx) = 0;
  //
  virtual uint16_t numObjects(void) const { return 1; }
  //
  virtual void serialize(PacketBufferAC & out_buf) const throw(SerializerException);
  //
  virtual DsrlzMode_e deserialize(PacketBufferAC & in_buf,
                                  DsrlzMode_e use_mode = dsmComplete) throw(SerializerException);
};

//Template class for solid packet construction.
template < class _Command /* : public SerializableObjIface */>
class USSGPacket_T : public USSGPacketAC {
protected:
  // ------------------------------------------------------
  // -- SerializablePacketIface interface implementation
  // ------------------------------------------------------
  virtual const SerializableObjIface & at(uint16_t use_idx) const { return _Cmd; }
  virtual SerializableObjIface & at(uint16_t use_idx) { return _Cmd; }

public:
  _Command _Cmd;

  USSGPacket_T() : USSGPacketAC()
  { }
  //
  ~USSGPacket_T()
  { }
};

} //interaction
} //ussman
} //smsc

#endif /* __SMSC_USS_GATEWAY_MESSAGES_FMT */
