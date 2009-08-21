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

namespace smsc  {
namespace inman {
namespace interaction {

class SerializerException : public CustomException {
public:
    typedef enum { invPacket = 1, invObject, invObjData } ErrorClass;
    SerializerException(const char * msg,
                        ErrorClass ex_class = SerializerException::invObjData,
                        const char * desc = NULL)
        : CustomException((int)ex_class, msg, desc)
    { setExcId("SerializerException"); }

    ~SerializerException() throw()
    { }

    ErrorClass getErrClass(void) const { return (ErrorClass)errCode; }
};


typedef smsc::core::buffers::ExtendingBuffer<unsigned char,1024> ObjectBuffer;
/*
 * BYTE ORDER:   network << host   - sending
 *               network >> host   - recieving
 */
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

/* NOTE: on all platforms time_t is a signed integer type holding at least 4 bytes!!!
 */
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


class SerializablePacketAC : public std::vector<SerializableObjectAC*>  {
protected:
    std::vector<bool>   ownObj;

public:
    virtual void serialize(ObjectBuffer& out_buf) throw(SerializerException) = 0;

    void Resize(unsigned sz)
    {
        resize(sz, NULL);
        ownObj.resize(sz, false);
    }

    //NOTE: it's a caller responsibility to check for index range!!!
    SerializableObjectAC* releaseObj(unsigned idx)
    {
        SerializableObjectAC* pObj = at(idx);
        at(idx) = NULL; ownObj[idx] = false;
        return pObj;
    }
    //Deletes old object if any.
    void resetObj(unsigned idx)
    { 
        if (ownObj[idx] && at(idx)) {
            delete at(idx); at(idx) = NULL; ownObj[idx] = false;
        } 
    }
    //Returns object Id, or zero in case of error
    unsigned short referObj(unsigned idx, SerializableObjectAC & use_obj)
    {
        resetObj(idx);
        return (!(at(idx) = &use_obj) ? 0 : use_obj.Id());
    }
    //Returns object Id, or zero in case of error
    unsigned short assignObj(unsigned idx, SerializableObjectAC * use_obj)
    {
        resetObj(idx);
        unsigned short objId;
        ownObj[idx] = (objId = (!(at(idx) = use_obj)) ? 0 : use_obj->Id()) ? true : false;
        return objId;
    }

    virtual ~SerializablePacketAC()
    {
        for (unsigned i = 0; i < size(); i++)
            resetObj(i);
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

