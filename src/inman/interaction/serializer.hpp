#ident "$Id$"

#ifndef __SMSC_INMAN_INTERACTION_SERIALIZER__
#define __SMSC_INMAN_INTERACTION_SERIALIZER__

#include <vector>

#include "core/buffers/TmpBuf.hpp"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"

using smsc::logger::Logger;
using smsc::core::network::Socket;

namespace smsc  {
namespace inman {
namespace interaction {

typedef smsc::core::buffers::TmpBuf<char,2048> ObjectBuffer;


/*
 * NOTE: its considered that vectors/strings passed as arguments to operators defined
 * below have length not grater than 255 chars! 
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

  inline ObjectBuffer& operator<<(ObjectBuffer& buf,const unsigned int& val)
  {
    unsigned int nval=ntohl(val);
    buf.Append((char*)&nval,4);
    return buf;
  }

  inline ObjectBuffer& operator>>(ObjectBuffer& buf,unsigned int& val)
  {
    buf.Read((char*)&val,4);
    val=htonl(val);
    return buf;
  }

  inline ObjectBuffer& operator<<(ObjectBuffer& buf,const int& val)
  {
    unsigned int nval=ntohl(val);
    buf.Append((char*)&nval,4);
    return buf;
  }

  inline ObjectBuffer& operator>>(ObjectBuffer& buf,int& val)
  {
    buf.Read((char*)&val,4);
    val=htonl(val);
    return buf;
  }

  inline ObjectBuffer& operator<<(ObjectBuffer& buf,time_t val)
  {
    val=ntohl(val);
    buf.Append((char*)&val,4);
    return buf;
  }

  inline ObjectBuffer& operator>>(ObjectBuffer& buf,time_t& val)
  {
    buf.Read((char*)&val,4);
    val=htonl(val);
    return buf;
  }

  inline ObjectBuffer& operator<<(ObjectBuffer& buf,unsigned short val)
  {
    unsigned short nval=ntohs(val);
    buf.Append((char*)&nval,2);
    return buf;
  }

  inline ObjectBuffer& operator>>(ObjectBuffer& buf,unsigned short& val)
  {
    buf.Read((char*)&val,2);
    val=htons(val);
    return buf;
  }


  inline ObjectBuffer& operator<<(ObjectBuffer& buf,const unsigned char& val)
  {
    buf.Append((char*)&val,1);
    return buf;
  }

  inline ObjectBuffer& operator>>(ObjectBuffer& buf,unsigned char& val)
  {
    buf.Read((char*)&val,1);
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
        struct PipeFormat {
            enum { straightData = 0, lengthPrefixed = 1 };
        };

        ObjectPipe(Socket* sock, SerializerITF * serializer);
        ObjectPipe(Socket* sock, SerializerITF * serializer, unsigned pipe_format);
        ~ObjectPipe();

        SerializableObject* receive();
        void    send(SerializableObject* obj);
    private:
        Socket*     socket;
        Logger*     logger;
        unsigned    _format;
        SerializerITF * _objSerializer;
};

} //interaction
} //inman
} //smsc

#endif
