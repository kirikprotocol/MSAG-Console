#ident "$Id$"

#ifndef __SMSC_INMAN_INTERACTION_SERIALIZER__
#define __SMSC_INMAN_INTERACTION_SERIALIZER__

#include <vector>

#include "inman/common/errors.hpp"
#include "inman/common/util.hpp"
#include "inman/common/factory.hpp"
#include "core/buffers/TmpBuf.hpp"

#include "core/network/Socket.hpp"

using std::runtime_error;

using smsc::inman::common::format;
using smsc::inman::common::dump;
using smsc::inman::common::FactoryT;
using smsc::logger::Logger;
using smsc::core::network::Socket;

namespace smsc  {
namespace inman {
namespace interaction {

typedef smsc::core::buffers::TmpBuf<char,2048> ObjectBuffer;


  inline ObjectBuffer& operator<<(ObjectBuffer& buf,const char* str)
  {
    unsigned char len=strlen(str);
    buf.Append((char*)&len,1);
    buf.Append(str,len);
    return buf;
  }

  struct SmallCharBuf
  {
    char* buf;
    int len;
    SmallCharBuf(char* argBuf,int argLen):buf(argBuf),len(argLen){}
  };

  inline ObjectBuffer& operator>>(ObjectBuffer& buf,SmallCharBuf& scb)
  {
    unsigned char len;
    buf.Read((char*)&len,1);
    if(len>scb.len) throw runtime_error( format("Attempt to read %d byte in buffer with size %d",(int)len,(int)scb.len) );
    buf.Read(scb.buf,len);
    scb.buf[len]=0;
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
	friend class Serializer;
    protected:

    	virtual void load(ObjectBuffer& in)  = 0;
    	virtual void save(ObjectBuffer& out) = 0;
};

class ObjectPipe
{
	public:
		ObjectPipe(Socket* sock);
		~ObjectPipe();

		SerializableObject* receive();
		void send(SerializableObject* obj);
	private:
		Socket* socket;
		Logger* logger;
};


class Serializer : public FactoryT< USHORT_T, SerializableObject >
{

	public:
		enum { FORMAT_VERSION = 0x0001 };

		virtual ~Serializer();

		SerializableObject* deserialize(ObjectBuffer&);
		void				serialize(SerializableObject*, ObjectBuffer& out);

		static Serializer* getInstance();

	protected:
		Serializer();


};

}
}
}

#endif
