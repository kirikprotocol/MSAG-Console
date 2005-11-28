#ident "$Id$"

#ifndef __SMSC_INMAN_INTERACTION_SERIALIZER__
#define __SMSC_INMAN_INTERACTION_SERIALIZER__

#include <vector>

#include "core/buffers/TmpBuf.hpp"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"
#include "store/Uint64Converter.h"

using smsc::logger::Logger;
using smsc::core::network::Socket;
using smsc::util::Uint64Converter;

namespace smsc  {
namespace inman {
namespace interaction {

typedef smsc::core::buffers::TmpBuf<char,2048> ObjectBuffer;


/*
 * NOTE: its considered that vectors/strings passed as arguments to operators defined
 * below have length not grater than 255 chars! 
 *
 * BYTE ORDER:   network << host   - sending
 *               network >> host   - recieving
 */
inline ObjectBuffer& operator<<(ObjectBuffer& buf, const std::vector<unsigned char>& arr)
{
    unsigned char len = arr.size();
    buf.Append((char*)&len, 1);
    buf.Append((char*)&arr[0], len);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, std::vector<unsigned char>& arr )
{
    unsigned char len;
    buf.Read((char*)&len, 1);
    unsigned char* arrBuf = new unsigned char[len];
    buf.Read((char*)arrBuf, len);
    arr.assign(arrBuf, arrBuf + len);
    return buf;
}

inline ObjectBuffer& operator<<(ObjectBuffer& buf, const std::string& str)
{
    unsigned char len = str.size();
    buf.Append((char*)&len,1);
    buf.Append(str.c_str(),len);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, std::string& str )
{
    unsigned char len;
    buf.Read((char*)&len,1);
    char* strBuf = new char[len + 1];
    buf.Read(strBuf,len);
    str.assign( strBuf, strBuf + len );
    return buf;
}


inline ObjectBuffer& operator<<(ObjectBuffer& buf, const uint64_t& val)
{
    uint64_t nval = Uint64Converter::toNetworkOrder(val);
    buf.Append((char*)&nval, 8);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, uint64_t & val)
{
    uint64_t nval;
    buf.Read((char*)&nval, 8);
    val = Uint64Converter::toHostOrder(nval);
    return buf;
}


inline ObjectBuffer& operator<<(ObjectBuffer& buf, const uint32_t & val)
{
    uint32_t nval = htonl(val);
    buf.Append((char*)&nval, 4);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, uint32_t & val)
{
    buf.Read((char*)&val, 4);
    val = ntohl(val);
    return buf;
}

inline ObjectBuffer& operator<<(ObjectBuffer& buf, const int32_t& val)
{
    uint32_t nval= htonl((uint32_t)val);
    buf.Append((char*)&nval, 4);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, int32_t& val)
{
    buf.Read((char*)&val, 4);
    val = (int32_t)ntohl((uint32_t)val);
    return buf;
}

/* NOTE: on all platforms time_t is a signed integer type holding at least 4 bytes!!!
 */
inline ObjectBuffer& operator<<(ObjectBuffer& buf, const time_t& val)
{
    uint32_t nval = htonl((uint32_t)val);
    buf.Append((char*)&nval, 4);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, time_t& val)
{   
    uint32_t nval;
    buf.Read((char*)&nval, 4);
    val = (time_t)ntohl(nval);
    return buf;
}

inline ObjectBuffer& operator<<(ObjectBuffer& buf, const unsigned short& val)
{
    unsigned short nval = htons(val);
    buf.Append((char*)&nval, 2);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, unsigned short& val)
{
    buf.Read((char*)&val, 2);
    val = ntohs(val);
    return buf;
}


inline ObjectBuffer& operator<<(ObjectBuffer& buf, const unsigned char& val)
{
    buf.Append((char*)&val, 1);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, unsigned char& val)
{
    buf.Read((char*)&val, 1);
    return buf;
}

inline ObjectBuffer& operator<<(ObjectBuffer& buf, const bool& val)
{
    unsigned char nval = (unsigned char)val;
    buf.Append((char*)&nval, 1);
    return buf;
}
inline ObjectBuffer& operator>>(ObjectBuffer& buf, bool& val)
{
    unsigned char nval;
    buf.Read((char*)&nval, 1);
    val = nval ? true : false;
    return buf;
}



class SerializableObject
{
    public:
        SerializableObject() : dialogId(0), objectId(0) { }
        virtual ~SerializableObject() { }

        void setDialogId(unsigned int id) { dialogId = id;  }
        unsigned int getDialogId() const { return dialogId; }

        void setObjectId(unsigned short id) { objectId = id;  }
        unsigned short getObjectId() const { return objectId; }

        virtual void load(ObjectBuffer& in)  = 0;
        virtual void save(ObjectBuffer& out) = 0;

    protected:
        unsigned int   dialogId; //unique id of TCP dialog, to which object belongs to
        unsigned short objectId; //unique id of object 
};

//serializer interface:
struct SerializerITF {
    virtual SerializableObject* deserialize(ObjectBuffer&) = 0;
    virtual void                serialize(SerializableObject*, ObjectBuffer& out) = 0;
};


//serializes and sends object over TCP socket
class ObjectPipe
{
    public:
        typedef enum { frmStraightData = 0, frmLengthPrefixed = 1 } PipeFormat;

        ObjectPipe(Socket* sock, SerializerITF * serializer, Logger* uselog = NULL);
        ObjectPipe(Socket* sock, SerializerITF * serializer,
                   PipeFormat pipe_format, Logger* uselog = NULL);
        ~ObjectPipe();
        void    setPipeFormat(PipeFormat frm);
        void    setLogger(Logger* newlog);

        SerializableObject* receive();
        void    send(SerializableObject* obj);
    private:
        Socket*     socket;
        Logger*     logger;
        PipeFormat   _format;
        SerializerITF * _objSerializer;
};

} //interaction
} //inman
} //smsc

#endif
