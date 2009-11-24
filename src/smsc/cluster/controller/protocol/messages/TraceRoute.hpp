#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_TRACEROUTE_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_TRACEROUTE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) TraceRoute version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class TraceRoute{
public:
  TraceRoute()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    dstFlag=false;
    srcFlag=false;
    srcSysIdFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(dstFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="dst=";
      rv+=dst;
    }
    if(srcFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="src=";
      rv+=src;
    }
    if(srcSysIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="srcSysId=";
      rv+=srcSysId;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(dstFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(dst);
    }
    if(srcFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(src);
    }
    if(srcSysIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(srcSysId);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getDst()const
  {
    if(!dstFlag)
    {
      throw protogen::framework::FieldIsNullException("dst");
    }
    return dst;
  }
  void setDst(const std::string& value)
  {
    dst=value;
    dstFlag=true;
  }
  bool hasDst()const
  {
    return dstFlag;
  }
  const std::string& getSrc()const
  {
    if(!srcFlag)
    {
      throw protogen::framework::FieldIsNullException("src");
    }
    return src;
  }
  void setSrc(const std::string& value)
  {
    src=value;
    srcFlag=true;
  }
  bool hasSrc()const
  {
    return srcFlag;
  }
  const std::string& getSrcSysId()const
  {
    if(!srcSysIdFlag)
    {
      throw protogen::framework::FieldIsNullException("srcSysId");
    }
    return srcSysId;
  }
  void setSrcSysId(const std::string& value)
  {
    srcSysId=value;
    srcSysIdFlag=true;
  }
  bool hasSrcSysId()const
  {
    return srcSysIdFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!dstFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("dst");
    }
    if(!srcFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("src");
    }
    if(!srcSysIdFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("srcSysId");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(dstTag);
    ds.writeStrLV(dst);
    ds.writeTag(srcTag);
    ds.writeStrLV(src);
    ds.writeTag(srcSysIdTag);
    ds.writeStrLV(srcSysId);
    ds.writeTag(DataStream::endOfMessage_tag);
  }

  template <class DataStream>
  void deserialize(DataStream& ds)
  {
    Clear();
    bool endOfMessage=false;
    //uint8_t rdVersionMajor=ds.readByte();
    //uint8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("TraceRoute");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case dstTag:
        {
          if(dstFlag)
          {
            throw protogen::framework::DuplicateFieldException("dst");
          }
          dst=ds.readStrLV();
          dstFlag=true;
        }break;
        case srcTag:
        {
          if(srcFlag)
          {
            throw protogen::framework::DuplicateFieldException("src");
          }
          src=ds.readStrLV();
          srcFlag=true;
        }break;
        case srcSysIdTag:
        {
          if(srcSysIdFlag)
          {
            throw protogen::framework::DuplicateFieldException("srcSysId");
          }
          srcSysId=ds.readStrLV();
          srcSysIdFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("TraceRoute",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!dstFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("dst");
    }
    if(!srcFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("src");
    }
    if(!srcSysIdFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("srcSysId");
    }

  }

  uint32_t getSeqNum()const
  {
    return seqNum;
  }

  void setSeqNum(uint32_t value)
  {
    seqNum=value;
  }

protected:
  //static const uint8_t versionMajor=1;
  //static const uint8_t versionMinor=0;

  static const uint32_t dstTag=1;
  static const uint32_t srcTag=2;
  static const uint32_t srcSysIdTag=3;

  uint32_t seqNum;

  std::string dst;
  std::string src;
  std::string srcSysId;

  bool dstFlag;
  bool srcFlag;
  bool srcSysIdFlag;
};

}
}
}
}
}
#endif
