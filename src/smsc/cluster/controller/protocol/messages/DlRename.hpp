#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLRENAME_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLRENAME_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) DlRename version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class DlRename{
public:
  DlRename()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    srcDlNameFlag=false;
    dstDlNameFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(srcDlNameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="srcDlName=";
      rv+=srcDlName;
    }
    if(dstDlNameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="dstDlName=";
      rv+=dstDlName;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(srcDlNameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(srcDlName);
    }
    if(dstDlNameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(dstDlName);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getSrcDlName()const
  {
    if(!srcDlNameFlag)
    {
      throw protogen::framework::FieldIsNullException("srcDlName");
    }
    return srcDlName;
  }
  void setSrcDlName(const std::string& value)
  {
    srcDlName=value;
    srcDlNameFlag=true;
  }
  bool hasSrcDlName()const
  {
    return srcDlNameFlag;
  }
  const std::string& getDstDlName()const
  {
    if(!dstDlNameFlag)
    {
      throw protogen::framework::FieldIsNullException("dstDlName");
    }
    return dstDlName;
  }
  void setDstDlName(const std::string& value)
  {
    dstDlName=value;
    dstDlNameFlag=true;
  }
  bool hasDstDlName()const
  {
    return dstDlNameFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!srcDlNameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("srcDlName");
    }
    if(!dstDlNameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("dstDlName");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(srcDlNameTag);
    ds.writeStrLV(srcDlName);
    ds.writeTag(dstDlNameTag);
    ds.writeStrLV(dstDlName);
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
    //  throw protogen::framework::IncompatibleVersionException("DlRename");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case srcDlNameTag:
        {
          if(srcDlNameFlag)
          {
            throw protogen::framework::DuplicateFieldException("srcDlName");
          }
          srcDlName=ds.readStrLV();
          srcDlNameFlag=true;
        }break;
        case dstDlNameTag:
        {
          if(dstDlNameFlag)
          {
            throw protogen::framework::DuplicateFieldException("dstDlName");
          }
          dstDlName=ds.readStrLV();
          dstDlNameFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DlRename",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!srcDlNameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("srcDlName");
    }
    if(!dstDlNameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("dstDlName");
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

  static const uint32_t srcDlNameTag=1;
  static const uint32_t dstDlNameTag=2;

  uint32_t seqNum;

  std::string srcDlName;
  std::string dstDlName;

  bool srcDlNameFlag;
  bool dstDlNameFlag;
};

}
}
}
}
}
#endif
