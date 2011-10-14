#ifndef BUFFER_SERIALIZATION_DECLARATIONS
#define BUFFER_SERIALIZATION_DECLARATIONS

#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "util/int.h"
#include <stdexcept>
#include <string>
#include "util/Uint64Converter.h"
#include "core/buffers/FixedLengthString.hpp"

namespace smsc {
namespace util {

class SerializationBuffer
{
public:
  explicit SerializationBuffer(int size=0)
  {
    if(size>0)
    {
      buffer=new char[size];
      bufferSize=size;
    }else
    {
      buffer=0;
      bufferSize=0;
    }
    bufferPos=0;
    bufferOwned=true;
  }
  ~SerializationBuffer()
  {
    if(bufferOwned && buffer)
    {
      delete [] buffer;
    }
  }
  void Write(const void* buf,size_t sz)
  {
    resize((uint32_t)(bufferPos+sz));
    memcpy(buffer+bufferPos,buf,sz);
    bufferPos+=sz;
  }
  void Read(void* buf,size_t sz)
  {
    if(bufferPos+sz>bufferSize)throw std::runtime_error("Attempt to read beyond buffer");
    memcpy(buf,buffer+bufferPos,sz);
    bufferPos+=sz;
  }
  template <class T>
  void XWrite(const T& t)
  {
    Write(&t,sizeof(T));
  }
  template <class T>
  void XRead(T& t)
  {
    return Read(&t,sizeof(T));
  }
  uint64_t ReadInt64()
  {
    uint64_t t;
    XRead(t);
    return t;
  }

  uint64_t ReadNetInt64()
  {
    uint32_t h=ReadNetInt32();
    uint32_t l=ReadNetInt32();
    return (((uint64_t)h)<<32)|l;
  }

  uint32_t ReadInt32()
  {
    uint32_t t;
    XRead(t);
    return t;
  }

  uint32_t ReadNetInt32()
  {
    return ntohl(ReadInt32());
  }

  uint16_t ReadInt16()
  {
    uint16_t t;
    XRead(t);
    return t;
  }
  uint16_t ReadNetInt16()
  {
    return ntohs(ReadInt16());
  }

  uint8_t ReadByte()
  {
    uint8_t b;
    XRead(b);
    return b;
  }

  void WriteInt32(uint32_t t)
  {
    XWrite(t);
  }
  void WriteNetInt32(uint32_t t)
  {
    WriteInt32(htonl(t));
  }

  void WriteInt16(uint16_t t)
  {
    XWrite(t);
  }
  void WriteNetInt16(uint16_t t)
  {
    WriteInt16(htons(t));
  }

  void WriteInt64(uint64_t t)
  {
    XWrite(t);
  }
  void WriteNetInt64(uint64_t t)
  {
    uint32_t h=htonl((uint32_t)((t>>32)&0xFFFFFFFFUL));
    uint32_t l=htonl((uint32_t)(t&0xFFFFFFFFUL));
    XWrite(h);
    XWrite(l);
  }

  void WriteByte(uint8_t t)
  {
    XWrite(t);
  }

  template <size_t SZ>
  void WriteFixedString(const char (&str)[SZ])
  {
    char buf[SZ]={0,};
    memcpy(buf,str,std::min((size_t)SZ,strlen(str)));
    Write(buf,SZ);
  }

  template <size_t SZ>
  void WriteFixedString(const std::string& str)
  {
    char buf[SZ]={0,};
    memcpy(buf,str.c_str(),std::min((size_t)SZ,str.length()));
    Write(buf,SZ);
  }

  template <unsigned int SZ>
  void ReadFixedString(char (&str)[SZ])
  {
    Read(str,SZ);
    str[SZ-1]=0;
  }

  template <unsigned int SZ>
  void ReadFixedString(std::string& str)
  {
    char buf[SZ+1];
    buf[SZ-1]=0;
    Read(buf,SZ);
    str=buf;
  }

  template <class StrLenType>
  void WriteString(const std::string& str)
  {
    if(str.length()!=(StrLenType)str.length())
    {
      throw std::runtime_error("WriteString: string length is too large for requested length type");
    }
    if(sizeof(StrLenType)==1)
    {
      WriteByte((uint8_t)str.length());
    }else if(sizeof(StrLenType)==2)
    {
      WriteNetInt16((uint16_t)str.length());
    }else
    {
      WriteNetInt32((uint32_t)str.length());
    }
    Write(str.c_str(),str.length());
  }
  template <class StrLenType>
  void ReadString(std::string& str)
  {
    int len;
    if(sizeof(StrLenType)==1)
    {
      len=ReadByte();
    }else if(sizeof(StrLenType)==2)
    {
      len=ReadNetInt16();
    }else
    {
      len=ReadNetInt32();
    }
    if(bufferPos+len>bufferSize)throw std::runtime_error("Attempt to read beyond buffer");
    str.assign(buffer+bufferPos,len);
    bufferPos+=len;
  }

  template <class StrLenType,size_t N>
  void WriteString(const smsc::core::buffers::FixedLengthString<N>& str)
  {
    if(str.length()!=(StrLenType)str.length())
    {
      throw std::runtime_error("WriteString: string length is too large for requested length type");
    }
    if(sizeof(StrLenType)==1)
    {
      WriteByte((uint8_t)str.length());
    }else if(sizeof(StrLenType)==2)
    {
      WriteNetInt16((uint16_t)str.length());
    }else
    {
      WriteNetInt32((uint32_t)str.length());
    }
    Write(str.c_str(),str.length());
  }

  template <class StrLenType,size_t N>
  void ReadString(smsc::core::buffers::FixedLengthString<N>& str)
  {
    size_t len;
    if(sizeof(StrLenType)==1)
    {
      len=ReadByte();
    }else if(sizeof(StrLenType)==2)
    {
      len=ReadNetInt16();
    }else
    {
      len=ReadNetInt32();
    }
    if(bufferPos+len>bufferSize)throw std::runtime_error("Attempt to read beyond buffer");
    str.assign(buffer+bufferPos,len);
    bufferPos+=(uint32_t)len;
  }

  void assign(const void* buf,uint32_t sz)
  {
    resize(sz);
    memcpy(buffer,buf,sz);
  }

  void setExternalBuffer(void* buf,uint32_t sz)
  {
    if(bufferOwned && buffer)
    {
      delete [] buffer;
    }
    bufferOwned=false;
    buffer=(char*)buf;
    bufferSize=sz;
    bufferPos=0;
  }

  void* releaseBuffer()
  {
    bufferOwned=false;
    return buffer;
  }

  void* getBuffer() const
  {
    return buffer;
  }

  uint32_t getBufferSize() const
  {
    return bufferSize;
  }
  uint32_t getPos()
  {
    return bufferPos;
  }
  void setPos(uint32_t newPos)
  {
    bufferPos=newPos;
  }

  void resize(uint32_t newSize)
  {
    if(newSize<=bufferSize)return;
    if(!bufferOwned)throw std::runtime_error("Attempt to write to released buffer");
    char* newBuf=new char[newSize];
    memcpy(newBuf,buffer,bufferSize);
    delete [] buffer;
    buffer=newBuf;
    bufferSize=newSize;
  }

protected:
  char* buffer;
  uint32_t bufferSize;
  uint32_t bufferPos;
  bool bufferOwned;
  SerializationBuffer(const SerializationBuffer&);
  void operator=(const SerializationBuffer&);
};

}
}
#endif // BUFFER_SERIALIZATION_DECLARATIONS
