#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLMEMGET_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLMEMGET_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) DlMemGet version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class DlMemGet{
public:
  DlMemGet()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    dlNameFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(dlNameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="dlName=";
      rv+=dlName;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(dlNameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(dlName);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getDlName()const
  {
    if(!dlNameFlag)
    {
      throw protogen::framework::FieldIsNullException("dlName");
    }
    return dlName;
  }
  void setDlName(const std::string& value)
  {
    dlName=value;
    dlNameFlag=true;
  }
  bool hasDlName()const
  {
    return dlNameFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!dlNameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("dlName");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(dlNameTag);
    ds.writeStrLV(dlName);
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
    //  throw protogen::framework::IncompatibleVersionException("DlMemGet");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case dlNameTag:
        {
          if(dlNameFlag)
          {
            throw protogen::framework::DuplicateFieldException("dlName");
          }
          dlName=ds.readStrLV();
          dlNameFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DlMemGet",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!dlNameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("dlName");
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

  static const uint32_t dlNameTag=1;

  uint32_t seqNum;

  std::string dlName;

  bool dlNameFlag;
};

}
}
}
}
}
#endif
