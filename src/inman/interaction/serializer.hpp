#ifndef __SMSC_INMAN_INTERACTION_SERIALIZER__
#ident "@(#)$Id$"
#define __SMSC_INMAN_INTERACTION_SERIALIZER__

#include <arpa/inet.h>

#include <vector>
#include <string>
#include <memory>

#include "util/Uint64Converter.h"
using smsc::util::Uint64Converter;

#include "util/Exception.hpp"
using smsc::util::format;
using smsc::util::CustomException;

#include "core/buffers/ExtendingBuf.hpp"
#include "core/buffers/FixedLengthString.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

class SerializerException : public CustomException {
public:
    enum ErrorClass { invPacket = 1, invObject, invObjData };
    SerializerException(const char * msg,
                        ErrorClass ex_class = SerializerException::invObjData,
                        const char * desc = NULL)
        : CustomException((int)ex_class, msg, desc)
    { setExcId("SerializerException"); }

    ~SerializerException() throw()
    { }

    ErrorClass getErrClass(void) const { return (ErrorClass)errCode; }
};


typedef smsc::core::buffers::ExtendingBuffer_T<unsigned char, unsigned, 1024> ObjectBuffer;
/*
 * BYTE ORDER:   network << host   - sending
 *               network >> host   - recieving
 */

/* -------------------------------------------------------------------------- *
 * FixedLengthString serialization
 * -------------------------------------------------------------------------- */

template <size_t _SizeTArg>
ObjectBuffer& operator<<(ObjectBuffer& buf, const smsc::core::buffers::FixedLengthString<_SizeTArg> & str)
{
  unsigned   len = (unsigned)str.length();
  do {
      unsigned char l7b = (unsigned char)(len & 0x7F);
      if (len >>= 7)
          l7b |= 0x80;
      buf.Append(&l7b, 1);
  } while (len);
  buf.Append((const unsigned char*)str.c_str(), (unsigned)str.length());
  return buf;
}

template <size_t _SizeTArg>
ObjectBuffer& operator>>(ObjectBuffer& buf, smsc::core::buffers::FixedLengthString<_SizeTArg> & str)
   throw(SerializerException)
{
  unsigned len = 0, i = 0;
  unsigned char l7b;
  do {
      if (buf.Read(&l7b, 1) < 1)
          throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                      SerializerException::invObjData,
                                      " >> fixedString: corrupted size");
      len |= ((unsigned)(l7b & 0x7F) << (7*i++));
  } while ((l7b >= 0x80) && (i < ((sizeof(unsigned)<<3)/7)));

  if ((l7b >= 0x80) || (len >= _SizeTArg))
    throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                              SerializerException::invObjData, 
                              ">> fixedString: size is too large");

  unsigned num2r = (len <= (_SizeTArg-1) ? len : (unsigned)(_SizeTArg-1));
  if (buf.Read((unsigned char*)str.str, num2r) < num2r)
      throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                  SerializerException::invObjData,
                                  " >> fixedString: corrupted data");
  str.str[num2r] = 0;
  return buf;
}

/* -------------------------------------------------------------------------- *
 * std::vector serialization
 * -------------------------------------------------------------------------- */

inline ObjectBuffer& operator<<(ObjectBuffer& buf, const std::vector<unsigned char>& arr)
{
    unsigned   len = (unsigned)arr.size();
    do {
        unsigned char l7b = (unsigned char)(len & 0x7F);
        if (len >>= 7)
            l7b |= 0x80;
        buf.Append(&l7b, 1);
    } while (len);
    buf.Append(&arr[0], (unsigned)arr.size());
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, std::vector<unsigned char>& arr ) throw(SerializerException)
{
    unsigned len = 0, i = 0;
    unsigned char l7b;
    do {
        if (buf.Read(&l7b, 1) < 1)
            throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                      SerializerException::invObjData, 
                                      ">> vector: corrupted size");
        len |= ((unsigned)(l7b & 0x7F) << (7*i++));
    } while ((l7b >= 0x80) && (i < ((sizeof(unsigned)<<3)/7)));

    if (l7b >= 0x80)
      throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                SerializerException::invObjData, 
                                ">> vector: too long size");

    unsigned oldLen = (unsigned)arr.size();
    arr.resize(len + oldLen);
    if (buf.Read(&arr[oldLen], len) < len)
        throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                    SerializerException::invObjData,
                                    " >> vector: corrupted data");
    return buf;
}

/* -------------------------------------------------------------------------- *
 * std::string serialization
 * -------------------------------------------------------------------------- */

inline ObjectBuffer& operator<<(ObjectBuffer& buf, const std::string& str)
{
    unsigned   len = (int)str.size();
    do {
        unsigned char l7b = (unsigned char)(len & 0x7F);
        if (len >>= 7)
            l7b |= 0x80;
        buf.Append(&l7b, 1);
    } while (len);
    buf.Append((const unsigned char*)str.c_str(), (unsigned)str.size());
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, std::string& str ) throw(SerializerException)
{
    unsigned len = 0, i = 0;
    unsigned char l7b;
    do {
        if (buf.Read(&l7b, 1) < 1)
            throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                        SerializerException::invObjData,
                                        " >> string: corrupted size");
        len |= ((unsigned)(l7b & 0x7F) << (7*i++));
    } while ((l7b >= 0x80) && (i < ((sizeof(unsigned)<<3)/7)));

    if (l7b >= 0x80)
      throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                SerializerException::invObjData, 
                                ">> string: too long size");

    while (len) {
        char strBuf[255 + 1];
        unsigned num2r = (len <= 255) ? len : 255;
        
        if (buf.Read((unsigned char*)strBuf, num2r) < num2r)
            throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                        SerializerException::invObjData,
                                        " >> string: corrupted data");
        else {
            strBuf[num2r] = 0;
            str += strBuf;
        }
        len -= num2r;
    }
    return buf;
}

/* -------------------------------------------------------------------------- *
 * unsigned ints serialization
 * -------------------------------------------------------------------------- */

inline ObjectBuffer& operator<<(ObjectBuffer& buf, const uint64_t& val)
{
    uint64_t nval = Uint64Converter::toNetworkOrder(val);
    buf.Append((unsigned char*)&nval, 8);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, uint64_t & val) throw(SerializerException)
{
    uint64_t nval;
    if (buf.Read((unsigned char*)&nval, 8) < 8)
        throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                    SerializerException::invObjData,
                                    " >> uint64_t: corrupted data");
    else
        val = Uint64Converter::toHostOrder(nval);
    return buf;
}


inline ObjectBuffer& operator<<(ObjectBuffer& buf, const uint32_t & val)
{
    uint32_t nval = htonl(val);
    buf.Append((unsigned char*)&nval, 4);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, uint32_t & val) throw(SerializerException)
{
    if (buf.Read((unsigned char*)&val, 4) < 4)
        throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                    SerializerException::invObjData,
                                    " >> uint32_t: corrupted data");
    else
        val = ntohl(val);
    return buf;
}

inline ObjectBuffer& operator<<(ObjectBuffer& buf, const unsigned short& val)
{
    unsigned short nval = htons(val);
    buf.Append((unsigned char*)&nval, 2);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, unsigned short& val) throw(SerializerException)
{
    if (buf.Read((unsigned char*)&val, 2) < 2)
        throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                SerializerException::invObjData,
                                " >> uint16_t: corrupted data");
    else
        val = ntohs(val);
    return buf;
}

/* -------------------------------------------------------------------------- *
 * signed ints serialization
 * -------------------------------------------------------------------------- */

inline ObjectBuffer& operator<<(ObjectBuffer& buf, const int32_t& val)
{
    uint32_t nval= htonl((uint32_t)val);
    buf.Append((unsigned char*)&nval, 4);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, int32_t& val) throw(SerializerException)
{
    if (buf.Read((unsigned char*)&val, 4) < 4)
        throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                    SerializerException::invObjData,
                                    " >> int32_t: corrupted data");
    else
        val = (int32_t)ntohl((uint32_t)val);
    return buf;
}

/* -------------------------------------------------------------------------- *
 * time_t serialization
 * NOTE: on all platforms time_t is a signed integer type holding at least 4 bytes !!!
 * -------------------------------------------------------------------------- */
inline ObjectBuffer& operator<<(ObjectBuffer& buf, const time_t& val)
{
    uint32_t nval = htonl((uint32_t)val);
    buf.Append((unsigned char*)&nval, 4);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, time_t& val) throw(SerializerException)
{   
    uint32_t nval;
    if (buf.Read((unsigned char*)&nval, 4) < 4)
        throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                    SerializerException::invObjData,
                                    " >> time_t: corrupted data");
    else
        val = (time_t)ntohl(nval);
    return buf;
}


/* -------------------------------------------------------------------------- *
 * char/bool serialization
 * -------------------------------------------------------------------------- */

inline ObjectBuffer& operator<<(ObjectBuffer& buf, const unsigned char& val)
{
    buf.Append(&val, 1);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, unsigned char& val) throw(SerializerException)
{
    if (buf.Read(&val, 1) < 1)
        throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                SerializerException::invObjData,
                                " >> uchar_t: corrupted data");
    return buf;
}

inline ObjectBuffer& operator<<(ObjectBuffer& buf, const bool& val)
{
    unsigned char nval = (unsigned char)val;
    buf.Append(&nval, 1);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, bool& val) throw(SerializerException)
{
    unsigned char nval;
    if (buf.Read(&nval, 1) < 1)
        throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                SerializerException::invObjData,
                                " >> bool: corrupted data");
    val = nval ? true : false;
    return buf;
}


/* -------------------------------------------------------------------------- *
 * 
 * -------------------------------------------------------------------------- */
class SerializableObjectAC {
public:
    SerializableObjectAC(unsigned short obj_id)
        : objectId(obj_id), dataBuf(NULL), ownBuf(false)
    { }
    //constructor for copying
    SerializableObjectAC(const SerializableObjectAC &org_obj)
        : objectId(org_obj.objectId), dataBuf(NULL), ownBuf(false)
    { 
        //if there is a ObjectBuffer with data for deferred
        //deserialization then copy it
        if (org_obj.getDataBuf()) 
            setDataBuf(org_obj.getDataBuf(), false);
    }
    virtual ~SerializableObjectAC() {  if (dataBuf && ownBuf) delete dataBuf; }

    unsigned short Id() const { return objectId; }

    virtual void load(ObjectBuffer& in) throw(SerializerException) = 0;
    virtual void save(ObjectBuffer& out) const = 0;

    void setDataBuf(ObjectBuffer * in, bool own = false)
    {
        if (dataBuf && ownBuf) {
            delete dataBuf;
            dataBuf = NULL;
        }
        if (own) {
            dataBuf = in;
        } else {
            dataBuf = new ObjectBuffer(in->getDataSize());
            *dataBuf = *in;
        }
        ownBuf = true;
    }
    ObjectBuffer * getDataBuf(void) const { return dataBuf; }

    void loadDataBuf(void) throw(SerializerException) { if (dataBuf) load(*dataBuf); }

protected:
    unsigned short objectId; //unique id of object 
    ObjectBuffer * dataBuf;  //data for deferred deserialization
    bool           ownBuf;   //free buffer while destroying object
};


/* -------------------------------------------------------------------------- *
 * 
 * -------------------------------------------------------------------------- */
class SerializablePacketAC /* vector<SerializableObjectAC*> */  {
protected:
    class Element {
    protected:
      bool                    _ownObj;
      SerializableObjectAC *  _pObj;

    public:
      Element() : _ownObj(false), _pObj(0)
      { }
      ~Element()
      {
        reset();
      }

      SerializableObjectAC * getObj(void) const
      {
        return _pObj;
      }
      void setObj(SerializableObjectAC *  use_obj)
      {
        _ownObj = true; _pObj = use_obj;
      }
      void refObj(SerializableObjectAC *  use_obj)
      {
        _ownObj = false; _pObj = use_obj;
      }

      SerializableObjectAC * release(void)
      {
        SerializableObjectAC * tmp = _pObj;
        _ownObj = false; _pObj = NULL;
        return tmp;
      }

      void reset(void)
      {
        if (_ownObj)
          delete _pObj;
        _ownObj = false; _pObj = NULL;
      }
    };

    virtual unsigned size(void) const = 0;
    virtual const Element & at(unsigned use_idx) const = 0;
    virtual Element & at(unsigned use_idx) = 0;

public:
    virtual void serialize(ObjectBuffer& out_buf) const throw(SerializerException) = 0;


    bool checkIndex(unsigned use_idx) const
    {
      return (use_idx < size());
    }
    SerializableObjectAC * getObj(unsigned use_idx) const
    {
      return checkIndex(use_idx) ? at(use_idx).getObj() : 0;
    }

    SerializableObjectAC * releaseObj(unsigned use_idx)
    {
      return checkIndex(use_idx) ? at(use_idx).release() : 0;
    }
    //Deletes old object if any.
    void resetObj(unsigned use_idx)
    { 
      if (checkIndex(use_idx))
        at(use_idx).reset();
    }
    //Returns false in case of invalid index
    bool referObj(unsigned use_idx, SerializableObjectAC & use_obj)
    {
      if (checkIndex(use_idx)) {
        at(use_idx).refObj(&use_obj);
        return true;
      }
      return false;
    }
    //Sets object and gives ownership of it
    //Returns false in case of invalid index
    bool assignObj(unsigned use_idx, SerializableObjectAC & use_obj)
    {
      if (checkIndex(use_idx)) {
        at(use_idx).setObj(&use_obj);
        return true;
      }
      return false;
    }

    virtual ~SerializablePacketAC()
    { }
};

template <unsigned _SizeTArg>
class SerializablePacket_T : public SerializablePacketAC {
private:
  Element  _elem[_SizeTArg];

protected:
  unsigned size(void) const { return _SizeTArg; }
  const Element & at(unsigned use_idx) const { return _elem[use_idx]; }
  virtual Element & at(unsigned use_idx) { return _elem[use_idx]; }

public:
  SerializablePacket_T()
  {
    for (unsigned i = 0; i < _SizeTArg; ++i)
      new (_elem + i)Element();
  }

  virtual ~SerializablePacket_T()
  {
    for (unsigned i = 0; i < _SizeTArg; ++i)
      _elem[i].~Element();
  }
};

//Serializer interface:
class SerializerITF {
protected:
    virtual ~SerializerITF() //forbid interface destruction
    { }

public:
    virtual SerializablePacketAC*
            deserialize(ObjectBuffer& in) const throw(SerializerException) = 0;
    //this method able to take ownership of ObjectBuffer, it's usefull
    //in case of partial or deferred deserialization
    virtual SerializablePacketAC*
            deserialize(std::auto_ptr<ObjectBuffer>& p_in) const throw(SerializerException) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_SERIALIZER__ */

