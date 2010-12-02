#ifndef __SAMPLESERIALIZER_HPP__
#define __SAMPLESERIALIZER_HPP__

#include <inttypes.h>
#include <memory.h>
#include <arpa/inet.h>
#include <vector>
#include <string>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "util/utf8.h"
#include <iterator>

namespace eyeline{
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

  typedef int8_t TagType;
  typedef int32_t LengthType;
  enum{tagTypeSize=sizeof(TagType)};
  enum{lengthTypeSize=3};
  static const TagType endOfMessage_tag=(TagType)0xff;

protected:
  int8_t htonX(int8_t val)
  {
    return val;
  }
  int16_t htonX(int16_t val)
  {
    return htons(val);
  }
  int32_t htonX(int32_t val)
  {
    return htonl(val);
  }
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
    resize(tagTypeSize);
    TagType netvalue=htonX(tag);
    memcpy(buf+pos,&netvalue,tagTypeSize);
    pos+=tagTypeSize;
  }
  /*
  static int getLengthSize(LengthType length)
  {
    return 3;
  }
  */
  void writeLength(LengthType length)
  {
    if(length>0xffffff)
    {
      throw eyeline::protogen::framework::FieldTooLong();
    }
    resize(3);
    uint8_t high=(length>>16);
    memcpy(buf+pos,&high,1);
    pos++;
    length&=0xffff;
    int16_t netvalue=htonX((int16_t)length);
    memcpy(buf+pos,&netvalue,2);
    pos+=2;
  }
  void writeByte(int8_t value)
  {
    resize(1);
    memcpy(buf+pos,&value,1);
    pos+=1;
  }
  void writeInt16(int16_t value)
  {
    resize(2);
    int16_t netvalue=htons(value);
    memcpy(buf+pos,&netvalue,2);
    pos+=2;
  }
  void writeInt32(int32_t value)
  {
    resize(4);
    int32_t netvalue=htonl(value);
    memcpy(buf+pos,&netvalue,4);
    pos+=4;
  }
  void writeInt64(int64_t value)
  {
    writeInt32((int32_t)(value>>32));
    writeInt32((int32_t)(value&0xffffffffu));
  }
  void writeStr(const std::string& value)
  {
    resize(value.length()+lengthTypeSize);
    writeLength((LengthType)value.length());
    memcpy(buf+pos,value.c_str(),value.length());
    pos+=value.length();
    /*
    std::string::const_iterator it=value.begin(),end=value.end();
    writeLength((LengthType)utf8::distance(it,end)*2);
    for(;it!=end;)
    {
      //writeInt16(btowc((unsigned char)value[i]));
      writeInt16(utf8::next(it,end));
    }
    */
  }

  void writeBool(bool value)
  {
    resize(1);
    int8_t data=value?1:0;
    memcpy(buf+pos,&data,1);
    pos+=1;
  }

  void writeByteLV(int8_t value)
  {
    writeLength(1);
    writeByte(value);
  }

  void writeInt16LV(int16_t value)
  {
    writeLength(2);
    writeInt16(value);
  }

  void writeInt32LV(int32_t value)
  {
    writeLength(4);
    writeInt32(value);
  }

  void writeInt64LV(int64_t value)
  {
    writeLength(8);
    writeInt32((int32_t)(value>>32));
    writeInt32((int32_t)(value&0xffffffffu));
  }

  void writeBoolLV(bool value)
  {
    writeLength(1);
    writeBool(value);
  }

  void writeStrLV(const std::string& value)
  {
    writeStr(value);
  }

  static LengthType fieldSize(bool)
  {
    return 1;
  }
  static LengthType fieldSize(int8_t)
  {
    return 1;
  }
  static LengthType fieldSize(int16_t)
  {
    return 2;
  }
  static LengthType fieldSize(int32_t)
  {
    return 4;
  }
  static LengthType fieldSize(int64_t)
  {
    return 8;
  }
  static LengthType fieldSize(const std::string& value)
  {
    return (LengthType)(value.length());
    //return (LengthType)(value.length()*2);
    //return (LengthType)(utf8::distance(value.begin(),value.end())*2);
  }

  template <class T>
  static LengthType fieldSize(const T& value)
  {
    return fieldSize(value.getValue());
  }

  static LengthType fieldSize(const std::vector<std::string>& value)
  {
    LengthType rv=0;
    for(std::vector<std::string>::const_iterator it=value.begin(),end=value.end();it!=end;++it)
    {
      rv+=lengthTypeSize+fieldSize(*it);
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
    if(pos+3>dataSize)
    {
      throw ReadBeyonEof();
    }
    LengthType rv=0;
    uint8_t high;
    memcpy(&high,buf+pos,1);
    pos++;
    rv=high;
    rv<<=16;
    uint16_t low;
    memcpy(&low,buf+pos,2);
    low=htonX(low);
    rv|=low;
    pos+=2;
    return rv;
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
  int8_t readByte()
  {
    if(pos+1>dataSize)
    {
      throw ReadBeyonEof();
    }
    int8_t rv;
    memcpy(&rv,buf+pos,1);
    pos++;
    return rv;
  }
  int16_t readInt16()
  {
    if(pos+2>dataSize)
    {
      throw ReadBeyonEof();
    }
    int16_t rv;
    memcpy(&rv,buf+pos,2);
    pos+=2;
    return htons(rv);
  }
  int32_t readInt32()
  {
    if(pos+4>dataSize)
    {
      throw ReadBeyonEof();
    }
    int32_t rv;
    memcpy(&rv,buf+pos,4);
    pos+=4;
    return htonl(rv);
  }
  int64_t readInt64()
  {
    if(pos+8>dataSize)
    {
      throw ReadBeyonEof();
    }
    int64_t rv;
    int32_t tmp1;
    int32_t tmp2;
    memcpy(&tmp1,buf+pos,4);
    pos+=4;
    memcpy(&tmp2,buf+pos,4);
    pos+=4;
    rv=htonl(tmp1);
    rv<<=32;
    rv|=htonl(tmp2);
    return rv;
  }
  bool readBool()
  {
    if(pos+1>dataSize)
    {
      throw ReadBeyonEof();
    }
    int8_t rv;
    memcpy(&rv,buf+pos,1);
    pos+=1;
    return rv==1;
  }

  std::string readStr()
  {
    int16_t length=readLength();
    if(pos+length>dataSize)
    {
      throw ReadBeyonEof();
    }
    std::string rv;
    rv.reserve(length);
    for(int i=0;i<length;i++)
    {
      //utf8::append(readInt16(),std::back_inserter(rv));
      rv.append(1,(char)readByte());
    }
    return rv;
  }

  std::string readStrLV()
  {
    int16_t length=readLength();
    if(pos+length>dataSize)
    {
      throw ReadBeyonEof();
    }
    std::string rv;
    //rv.reserve(length*2);
    for(int i=0;i<length;i++)
    {
      //utf8::append(readInt16(),std::back_inserter(rv));
      rv.append(1,(char)readByte());
    }
    return rv;
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

  int8_t readByteLV()
  {
    LengthType len=readLength();
    if(len!=1)
    {
      throw InvalidValueLength("byte",len);
    }
    return readByte();
  }


  int16_t readInt16LV()
  {
    LengthType len=readLength();
    if(len!=2)
    {
      throw InvalidValueLength("int16",len);
    }
    return readInt16();
  }

  int32_t readInt32LV()
  {
    LengthType len=readLength();
    if(len!=4)
    {
      throw InvalidValueLength("int32",len);
    }
    return readInt32();
  }

  int64_t readInt64LV()
  {
    LengthType len=readLength();
    if(len!=8)
    {
      throw InvalidValueLength("int64",len);
    }
    return readInt64();
  }


  void skip(int32_t bytes)
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
    size_t sz=ownedBuffer?pos:dataSize;
    for(size_t i=0;i<sz;i++)
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
  char* prepareBuffer(int32_t size)
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
}

#endif
