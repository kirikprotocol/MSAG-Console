#ifndef __SAMPLESERIALIZER_HPP__
#define __SAMPLESERIALIZER_HPP__

#include <inttypes.h>
#include <memory.h>
#include <arpa/inet.h>
#include <vector>
#include <string>
#include <wchar.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace protogen{
namespace framework{

class SerializerBuffer{
public:
  SerializerBuffer()
  {
    bufSize=0;
    buf=0;
    pos=0;
    dataSize=0;
    ownedBuffer=false;
  }

  SerializerBuffer(size_t sz)
  {
    bufSize=sz;
    buf=new char[sz];
    pos=0;
    dataSize=0;
    ownedBuffer=true;
  }

  ~SerializerBuffer()
  {
    if(ownedBuffer)
    {
      delete [] buf;
    }
  }

  typedef uint16_t TagType;
  typedef uint16_t LengthType;
  enum{tagTypeSize=sizeof(TagType)};
  enum{lengthTypeSize=sizeof(LengthType)};
  enum{endOfMessage_tag=0xffff};

protected:
  uint8_t htonX(uint8_t val)
  {
    return val;
  }
  uint16_t htonX(uint16_t val)
  {
    return htons(val);
  }
  uint32_t htonX(uint32_t val)
  {
    return htonl(val);
  }
public:
  void writeTag(TagType tag)
  {
    resize(2);
    TagType netvalue=htonX(tag);
    memcpy(buf+pos,&netvalue,2);
    pos+=2;
  }
  void writeLength(LengthType length)
  {
    resize(2);
    LengthType netvalue=htonX(length);
    memcpy(buf+pos,&netvalue,2);
    pos+=2;
  }
  void writeByte(uint8_t value)
  {
    memcpy(buf+pos,&value,1);
    pos+=1;
  }
  void writeInt16(uint16_t value)
  {
    resize(2);
    uint16_t netvalue=htons(value);
    memcpy(buf+pos,&netvalue,2);
    pos+=2;
  }
  void writeInt32(uint32_t value)
  {
    resize(4);
    uint32_t netvalue=htonl(value);
    memcpy(buf+pos,&netvalue,4);
    pos+=4;
  }
  void writeStr(const std::string& value)
  {
    resize(value.length()*2);
    //memcpy(buf+pos,value.c_str(),value.length());
    for(size_t i=0;i<value.length();i++)
    {
      writeInt16(btowc((unsigned char)value[i]));
    }
    //pos+=value.length();
  }

  void writeBool(bool value)
  {
    resize(1);
    uint8_t data=value?1:0;
    memcpy(buf+pos,&data,1);
    pos+=1;
  }

  void writeByteLV(uint8_t value)
  {
    writeLength(1);
    writeByte(value);
  }

  void writeInt16LV(uint16_t value)
  {
    writeLength(2);
    writeInt16(value);
  }

  void writeInt32LV(uint32_t value)
  {
    writeLength(4);
    writeInt32(value);
  }

  void writeInt64LV(uint64_t value)
  {
    writeLength(8);
    writeInt32((uint32_t)(value>>32));
    writeInt32((uint32_t)(value&0xffffffffu));
  }

  void writeBoolLV(bool value)
  {
    writeLength(1);
    writeBool(value);
  }

  void writeStrLV(const std::string& value)
  {
    writeLength((LengthType)value.length()*2);
    writeStr(value);
  }

  void writeStrLstLV(const std::vector<std::string>& value)
  {
    writeLength(fieldSize(value));
    for(std::vector<std::string>::const_iterator it=value.begin(),end=value.end();it!=end;it++)
    {
      writeStrLV(*it);
    }
  }

  static LengthType fieldSize(bool)
  {
    return 1;
  }
  static LengthType fieldSize(uint8_t)
  {
    return 1;
  }
  static LengthType fieldSize(uint16_t)
  {
    return 2;
  }
  static LengthType fieldSize(uint32_t)
  {
    return 4;
  }
  static LengthType fieldSize(uint64_t)
  {
    return 8;
  }
  static LengthType fieldSize(const std::string& value)
  {
    return (LengthType)value.length()*2;
  }

  static LengthType fieldSize(const std::vector<std::string>& value)
  {
    LengthType rv=0;
    for(std::vector<std::string>::const_iterator it=value.begin(),end=value.end();it!=end;it++)
    {
      rv+=fieldSize(*it);
    }
    return rv;
  }

  template <typename T>
  static LengthType fieldSize(const std::vector<T>& value)
  {
    return (LengthType)(value.empty()?0:value.size()*fieldSize(value.front()));
    /*LengthType rv=0;
    for(typename std::vector<T>::const_iterator it=value.begin(),end=value.end();it!=end;it++)
    {
      rv+=fieldSize(*it);
    }
    return rv;*/
  }

  bool hasMoreData()
  {
    return pos<dataSize;
  }

  LengthType readLength()
  {
    if(pos+lengthTypeSize>dataSize)
    {
      throw ReadBeyonEof();
    }
    LengthType rv;
    memcpy(&rv,buf+pos,lengthTypeSize);
    pos+=lengthTypeSize;
    return htonX(rv);
  }
  TagType readTag()
  {
    if(pos+tagTypeSize>dataSize)
    {
      throw ReadBeyonEof();
    }
    TagType rv;
    memcpy(&rv,buf+pos,tagTypeSize);
    pos+=tagTypeSize;
    return htonX(rv);
  }
  uint8_t readByte()
  {
    if(pos+1>dataSize)
    {
      throw ReadBeyonEof();
    }
    uint8_t rv;
    memcpy(&rv,buf+pos,1);
    pos++;
    return rv;
  }
  uint16_t readInt16()
  {
    if(pos+2>dataSize)
    {
      throw ReadBeyonEof();
    }
    uint16_t rv;
    memcpy(&rv,buf+pos,2);
    pos+=2;
    return htons(rv);
  }
  uint32_t readInt32()
  {
    if(pos+4>dataSize)
    {
      throw ReadBeyonEof();
    }
    uint32_t rv;
    memcpy(&rv,buf+pos,4);
    pos+=4;
    return htonl(rv);
  }
  uint64_t readInt64()
  {
    if(pos+8>dataSize)
    {
      throw ReadBeyonEof();
    }
    uint64_t rv;
    uint32_t tmp1;
    uint32_t tmp2;
    memcpy(&tmp1,buf+pos,4);
    pos+=4;
    memcpy(&tmp2,buf+pos,4);
    pos+=4;
    rv=tmp1;
    rv<<=32;
    rv|=tmp2;
    return rv;
  }
  bool readBool()
  {
    if(pos+1>dataSize)
    {
      throw ReadBeyonEof();
    }
    uint8_t rv;
    memcpy(&rv,buf+pos,1);
    pos+=1;
    return rv==1;
  }

  std::string readStrLV()
  {
    uint16_t length=readLength();
    if(pos+length>dataSize)
    {
      throw ReadBeyonEof();
    }
    std::string rv;
    for(int i=0;i<length/2;i++)
    {
      rv+=wctob(readInt16());
    }
    //rv.assign(buf+pos,length);
    //pos+=length;
    return rv;
  }

  void readStrLstLV(std::vector<std::string>& value)
  {
    uint16_t length=readLength();
    if(pos+length>dataSize)
    {
      throw ReadBeyonEof();
    }
    uint16_t rd=0;
    std::string val;
    while(rd<length)
    {
      val=readStrLV();
      value.push_back(val);
      rd+=fieldSize(val);
    }
  }


  bool readBoolLV()
  {
    LengthType len=readLength();
    if(len!=1)
    {
      throw InvalidValueLength("bool",len);
    }
    return readBool();
  }

  uint8_t readByteLV()
  {
    LengthType len=readLength();
    if(len!=1)
    {
      throw InvalidValueLength("byte",len);
    }
    return readByte();
  }


  uint16_t readInt16LV()
  {
    LengthType len=readLength();
    if(len!=2)
    {
      throw InvalidValueLength("int16",len);
    }
    return readInt16();
  }

  uint32_t readInt32LV()
  {
    LengthType len=readLength();
    if(len!=4)
    {
      throw InvalidValueLength("int32",len);
    }
    return readInt32();
  }

  uint64_t readInt64LV()
  {
    LengthType len=readLength();
    if(len!=8)
    {
      throw InvalidValueLength("uint64",len);
    }
    return readInt64();
  }


  void skip(uint32_t bytes)
  {
    if(pos+bytes>dataSize)
    {
      throw ReadBeyonEof();
    }
    pos+=bytes;
  }

  std::string getDump()
  {
    char buffer[32];
    std::string rv;
    for(size_t i=0;i<pos;i++)
    {
      sprintf(buffer,"%02x ",(unsigned int)(unsigned char)buf[i]);
      rv+=buffer;
    }
    return rv;
  }
  void rewind()
  {
    dataSize=pos;
    pos=0;
  }
  void setExternalData(const char* data,size_t size)
  {
    pos=0;
    if(ownedBuffer && buf)
    {
      delete [] buf;
    }
    buf=const_cast<char*>(data);
    dataSize=size;
    ownedBuffer=false;
  }
  char* prepareBuffer(uint32_t size)
  {
    pos=0;
    resize(size);
    dataSize=size;
    return buf;
  }

  const char* getBuffer()const
  {
    return buf;
  }
  size_t getDataWritten()const
  {
    return pos;
  }
  const char* detachBuffer()
  {
    ownedBuffer=false;
    return buf;
  }
protected:
  char* buf;
  size_t pos;
  size_t dataSize;
  size_t bufSize;
  bool ownedBuffer;
  void resize(size_t grow)
  {
    if(!ownedBuffer)
    {
      throw protogen::framework::WriteToReadonlyBuffer();
    }
    if(pos+grow>bufSize)
    {
      bufSize=bufSize*2+grow;
      char* newbuf=new char[bufSize];
      memcpy(newbuf,buf,pos);
      delete [] buf;
      buf=newbuf;
      ownedBuffer=true;
    }
  }
};

}
}

#endif
