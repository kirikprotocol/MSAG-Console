#ident "$Id$"

#ifndef __SMSC_INMAN_INTERACTION_SERIALIZER__
#define __SMSC_INMAN_INTERACTION_SERIALIZER__

#include <arpa/inet.h>

#include <vector>
#include <string>

#include "util/Uint64Converter.h"
#include "inman/common/errors.hpp"
#include "inman/common/SerialBuf.hpp"

using smsc::util::Uint64Converter;
using smsc::inman::common::CustomException;

namespace smsc  {
namespace inman {
namespace interaction {

typedef smsc::core::buffers::SerialBuffer<unsigned char,1024> ObjectBuffer;

class SerializerException : public CustomException
{
public:
    typedef enum { invPacket = 1, invObject, invObjData } ErrorClass;
/*
    SerializerException(std::string & msg,
                        ErrorClass ex_class = SerializerException::invObjData,
                        const char * desc = NULL)
        : CustomException(msg.c_str(), ex_class, desc)
    {
        setExcId("SerializerException");
    }
*/
    SerializerException(const char * msg,
                        ErrorClass ex_class = SerializerException::invObjData,
                        const char * desc = NULL)
        : CustomException(msg, ex_class, desc)
    {
        setExcId("SerializerException");
    }
    ~SerializerException() throw()
    {}

    ErrorClass getErrClass(void) const { return (ErrorClass)errCode; }
};

/*
 * BYTE ORDER:   network << host   - sending
 *               network >> host   - recieving
 */
inline ObjectBuffer& operator<<(ObjectBuffer& buf, const std::vector<unsigned char>& arr)
{
    int   len = (int)arr.size();
    do {
        unsigned char l7b = (unsigned char)(len & 0x7F);
        if (len >>= 7)
            l7b |= 0x80;
        buf.Append(&l7b, 1);
    } while (len);
    buf.Append(&arr[0], (int)arr.size());
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, std::vector<unsigned char>& arr ) throw(CustomException)
{
    int len = 0;
    unsigned char l7b;
    do {
        if (buf.Read(&l7b, 1) < 1)
            throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                      SerializerException::invObjData, 
                                      ">> vector: corrupted size");
        len = (len << 7) + (l7b & 0x7F);
    } while (l7b >= 0x80);

    int oldLen = arr.size();
    arr.resize(len + oldLen);
    if (buf.Read(&arr[oldLen], len) < len)
        throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                    SerializerException::invObjData,
                                    " >> vector: corrupted data");
    return buf;
}

inline ObjectBuffer& operator<<(ObjectBuffer& buf, const std::string& str)
{
    int   len = (int)str.size();
    do {
        unsigned char l7b = (unsigned char)(len & 0x7F);
        if (len >>= 7)
            l7b |= 0x80;
        buf.Append(&l7b, 1);
    } while (len);
    buf.Append((const unsigned char*)str.c_str(), (int)str.size());
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, std::string& str ) throw(CustomException)
{
    int len = 0;
    unsigned char l7b;
    do {
        if (buf.Read(&l7b, 1) < 1)
            throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                        SerializerException::invObjData,
                                        " >> string: corrupted size");
        len = (len << 7) + (l7b & 0x7F);
    } while (l7b >= 0x80);

    while (len) {
        char strBuf[255 + 1];
        int num2r = (len <= 255) ? len : 255;
        
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
inline ObjectBuffer& operator>>(ObjectBuffer& buf, uint64_t & val) throw(CustomException)
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
inline ObjectBuffer& operator>>(ObjectBuffer& buf, uint32_t & val) throw(CustomException)
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
inline ObjectBuffer& operator>>(ObjectBuffer& buf, int32_t& val) throw(CustomException)
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
inline ObjectBuffer& operator>>(ObjectBuffer& buf, time_t& val) throw(CustomException)
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
inline ObjectBuffer& operator>>(ObjectBuffer& buf, unsigned short& val) throw(CustomException)
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
inline ObjectBuffer& operator>>(ObjectBuffer& buf, unsigned char& val) throw(CustomException)
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
inline ObjectBuffer& operator>>(ObjectBuffer& buf, bool& val) throw(CustomException)
{
    unsigned char nval;
    if (buf.Read(&nval, 1) < 1)
        throw SerializerException(format("ObjectBuffer[pos: %u]", buf.getPos()).c_str(),
                                SerializerException::invObjData,
                                " >> bool: corrupted data");
    val = nval ? true : false;
    return buf;
}


class SerializableObject
{
public:
    SerializableObject() : dialogId(0), objectId(0), dataBuf(NULL), ownBuf(false) { }
    ~SerializableObject() {  if (dataBuf && ownBuf) delete dataBuf; }

    void setDialogId(unsigned int id) { dialogId = id;  }
    unsigned int getDialogId() const { return dialogId; }

    void setObjectId(unsigned short id) { objectId = id;  }
    unsigned short getObjectId() const { return objectId; }

    virtual void load(ObjectBuffer& in) throw(CustomException) = 0;
    virtual void save(ObjectBuffer& out) = 0;

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
    };
    ObjectBuffer * getDataBuf(void) const { return dataBuf; };

    void loadDataBuf(void) throw(CustomException) { if (dataBuf) load(*dataBuf); }

protected:
    unsigned int   dialogId; //unique id of TCP dialog, to which object belongs to
    unsigned short objectId; //unique id of object 
    ObjectBuffer * dataBuf;  //data for deferred deserialization
    bool           ownBuf;   //free buffer while destroying object
};

//Serializer interface:
struct SerializerITF {
    virtual SerializableObject*
            deserialize(ObjectBuffer& in) throw(CustomException) = 0;
    //this method able to take ownership of ObjectBuffer, it's usefull in case 
    //of partial deserialization (only packet prefix is parsed while
    //the deserialization of contained object is deferred)
    virtual SerializableObject*
            deserializeAndOwn(ObjectBuffer* in, bool ownBuf = true) throw(CustomException); 
    virtual void
            serialize(SerializableObject*, ObjectBuffer& out) = 0;
};

} //interaction
} //inman
} //smsc

#endif
