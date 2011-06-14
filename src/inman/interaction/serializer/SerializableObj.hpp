/* ************************************************************************** *
 * Interface definition of generic serializable object and packet.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INTERACTION_SERIALIZABLE_OBJ
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_SERIALIZABLE_OBJ

#include "inman/interaction/serializer/SerializerDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

/* -------------------------------------------------------------------------- *
 * 
 * -------------------------------------------------------------------------- */
class SerializableObjIface {
public:
  typedef uint16_t ident_type;

  ident_type Id(void) const { return _objectId; }

  // --------------------------------------------
  // -- SerializableObjIface interface methods
  // --------------------------------------------
  virtual void load(PacketBufferAC & in_buf) throw(SerializerException) = 0;
  virtual void save(PacketBufferAC & out_buf) const throw(SerializerException) = 0;

protected:
  ident_type    _objectId; //unique id of object 

  explicit SerializableObjIface(ident_type obj_id)
    : _objectId(obj_id)
  { }
  virtual ~SerializableObjIface()
  { }
};
typedef SerializableObjIface::ident_type SerializableObjID;

/* -------------------------------------------------------------------------- *
 * 
 * -------------------------------------------------------------------------- */
class SerializablePacketIface /* : vector<SerializableObjIface> */  {
protected:
  SerializablePacketIface()
  { }

  virtual ~SerializablePacketIface()
  { }

public:
  //Packet deserialization mode
  enum DsrlzMode_e {
    dsmComplete = 0 //whole packet deserialization
  , dsmPartial      //partial packet deserialization, deferred data remains in PacketBufferAC
  };

  // ------------------------------------------------------
  // -- SerializablePacketIface interface methods
  // ------------------------------------------------------
  virtual uint16_t numObjects(void) const = 0;
  //
  virtual const SerializableObjIface & at(uint16_t use_idx) const = 0;
  //
  virtual SerializableObjIface & at(uint16_t use_idx) = 0;
  //
  virtual void serialize(PacketBufferAC & out_buf) const
    throw(SerializerException) = 0;
  //
  virtual DsrlzMode_e deserialize(PacketBufferAC & in_buf, DsrlzMode_e use_mode = dsmComplete)
    throw(SerializerException) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_SERIALIZABLE_OBJ */

