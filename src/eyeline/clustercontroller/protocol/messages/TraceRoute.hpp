#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_TRACEROUTE_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_TRACEROUTE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) TraceRoute version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


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
    fileNameFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 8;
  }

  static std::string messageGetName()
  {
    return "TraceRoute";
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
    if(fileNameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="fileName=";
      rv+=fileName;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
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
    if(fileNameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(fileName);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getDst()const
  {
    if(!dstFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("dst");
    }
    return dst;
  }
  void setDst(const std::string& argValue)
  {
    dst=argValue;
    dstFlag=true;
  }
  std::string& getDstRef()
  {
    dstFlag=true;
    return dst;
  }
  bool hasDst()const
  {
    return dstFlag;
  }
  const std::string& getSrc()const
  {
    if(!srcFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("src");
    }
    return src;
  }
  void setSrc(const std::string& argValue)
  {
    src=argValue;
    srcFlag=true;
  }
  std::string& getSrcRef()
  {
    srcFlag=true;
    return src;
  }
  bool hasSrc()const
  {
    return srcFlag;
  }
  const std::string& getSrcSysId()const
  {
    if(!srcSysIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("srcSysId");
    }
    return srcSysId;
  }
  void setSrcSysId(const std::string& argValue)
  {
    srcSysId=argValue;
    srcSysIdFlag=true;
  }
  std::string& getSrcSysIdRef()
  {
    srcSysIdFlag=true;
    return srcSysId;
  }
  bool hasSrcSysId()const
  {
    return srcSysIdFlag;
  }
  const std::string& getFileName()const
  {
    if(!fileNameFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("fileName");
    }
    return fileName;
  }
  void setFileName(const std::string& argValue)
  {
    fileName=argValue;
    fileNameFlag=true;
  }
  std::string& getFileNameRef()
  {
    fileNameFlag=true;
    return fileName;
  }
  bool hasFileName()const
  {
    return fileNameFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!dstFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("dst");
    }
    if(!srcFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("src");
    }
    if(!srcSysIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("srcSysId");
    }
    if(!fileNameFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("fileName");
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
    ds.writeTag(fileNameTag);
    ds.writeStrLV(fileName); 
    ds.writeTag(DataStream::endOfMessage_tag);
  }

  template <class DataStream>
  void deserialize(DataStream& ds)
  {
    Clear();
    bool endOfMessage=false;
    //int8_t rdVersionMajor=ds.readByte();
    //int8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("TraceRoute");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case dstTag:
        {
          if(dstFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("dst");
          }
          dst=ds.readStrLV();
          dstFlag=true;
        }break;
        case srcTag:
        {
          if(srcFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("src");
          }
          src=ds.readStrLV();
          srcFlag=true;
        }break;
        case srcSysIdTag:
        {
          if(srcSysIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("srcSysId");
          }
          srcSysId=ds.readStrLV();
          srcSysIdFlag=true;
        }break;
        case fileNameTag:
        {
          if(fileNameFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("fileName");
          }
          fileName=ds.readStrLV();
          fileNameFlag=true;
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("dst");
    }
    if(!srcFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("src");
    }
    if(!srcSysIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("srcSysId");
    }
    if(!fileNameFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("fileName");
    }

  }

  int32_t messageGetSeqNum()const
  {
    return seqNum;
  }

  void messageSetSeqNum(int32_t argValue)
  {
    seqNum=argValue;
  }

 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t dstTag=1;
  static const int32_t srcTag=2;
  static const int32_t srcSysIdTag=3;
  static const int32_t fileNameTag=4;

  int32_t seqNum;

  std::string dst;
  std::string src;
  std::string srcSysId;
  std::string fileName;

  bool dstFlag;
  bool srcFlag;
  bool srcSysIdFlag;
  bool fileNameFlag;
};

}
}
}
}
#endif
