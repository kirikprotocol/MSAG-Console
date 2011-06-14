/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INTERACTION_SERIALIZER_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_SERIALIZER_DEFS

#include "util/Exception.hpp"

#include "inman/interaction/PacketBuffer.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

class SerializerException : public smsc::util::CustomException {
public:
  enum ErrorClass { invPacket = 1, invObject, invObjData };

  SerializerException() : smsc::util::CustomException()
  {
    setExcId("SerializerException");
  }
  //Generic serializer exception
  SerializerException(const char * msg,
                      ErrorClass ex_class = SerializerException::invObjData,
                      const char * desc = NULL)
    : smsc::util::CustomException((int)ex_class, msg, desc)
  {
    setExcId("SerializerException");
  }
  //
  ~SerializerException() throw()
  { }

  ErrorClass getErrClass(void) const { return static_cast<ErrorClass>(errorCode()); }
};

//Thrown if procesing of data at specific position of PacketBufferAC caused exception.
class BufferDataException : public SerializerException {
public:
  explicit BufferDataException(PacketBufferAC::size_type buff_pos,
                               const char * desc = NULL)
    : SerializerException()
  {
    char msgBuf[sizeof("PacketBuffer[pos: %u]") + sizeof(PacketBufferAC::size_type)*3];

    snprintf(msgBuf, sizeof(msgBuf)-1, "PacketBuffer[pos: %u]", buff_pos);
    fill(msgBuf, invObjData, desc);
  }
};

//Thrown if serialization of data to PacketBufferAC caused its overflow.
class BufferOverflowException : public SerializerException {
public:
  BufferOverflowException(const char * op_id, unsigned long num_bytes)
    : SerializerException()
  {
    fill(invObjData, "PacketBuffer overflow (<< %s %lu bytes)", op_id, num_bytes);
  }
};

//Thrown if deserialization of PacketBufferAC data to DataContaner caused its overflow.
class DataContainerOverflowException : public SerializerException {
public:
  DataContainerOverflowException(const char * cont_id, unsigned long num_bytes)
    : SerializerException()
  {
    fill(invObjData, ">> %s overflow (PacketBuffer %lu bytes)", cont_id, num_bytes);
  }
};


/* -------------------------------------------------------------------------- *
 * Generic serializer interface. 
 *  
 * BYTE ORDER:  network << host   - sending 
 *              network >> host   - recieving
 * -------------------------------------------------------------------------- */

//Serializes given data to PacketBufferAC
//template <class _DataContainerArg>
//PacketBufferAC &
//  operator<<(PacketBufferAC & buf, const _DataContainerArg & arr) throw(SerializerException);

//Deserializes data from PacketBufferAC to given DataContainer, adjusting buffer data
//size and position.
//Note: deserialized data is appended to given DataContainer, not overwrite it.
//template <class _DataContainerArg>
//PacketBufferAC &
//  operator>>(PacketBufferAC & buf, _DataContainerArg & arr) throw(SerializerException);

//Deserializes data from PacketBufferAC to given DataContainer, keepeing buffer intact.
//Note: deserialized data is appended to given DataContainer, not overwrite it.
//template <class _DataContainerArg>
//PacketBufferAC::size_type
//  operator>>(const PacketBufferAC & buf, _DataContainerArg & arr) throw(SerializerException);


//Incrementally deserializes data from PacketBufferAC to various DataContainer,
//keepeing buffer intact and maintaining current buffer position.
//Note: deserialized data is appended to given DataContainer, not overwrite it.
struct PacketBufferCursor {
  const PacketBufferAC &    _pckBuf;
  PacketBufferAC::size_type _curOfs;

  PacketBufferCursor(const PacketBufferAC & use_buf, PacketBufferAC::size_type use_ofs)
    : _pckBuf(use_buf), _curOfs(use_ofs)
  { }
  ~PacketBufferCursor()
  { }

  template <class _DataContainerArg>
    PacketBufferCursor & operator>>(_DataContainerArg & out_val) throw(SerializerException);
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_SERIALIZER_DEFS */

