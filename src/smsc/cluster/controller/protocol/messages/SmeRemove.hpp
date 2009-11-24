#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMEREMOVE_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMEREMOVE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) SmeRemove version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class SmeRemove{
public:
  SmeRemove()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    smeIdFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(smeIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="smeId=";
      rv+=smeId;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(smeIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(smeId);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getSmeId()const
  {
    if(!smeIdFlag)
    {
      throw protogen::framework::FieldIsNullException("smeId");
    }
    return smeId;
  }
  void setSmeId(const std::string& value)
  {
    smeId=value;
    smeIdFlag=true;
  }
  bool hasSmeId()const
  {
    return smeIdFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!smeIdFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("smeId");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(smeIdTag);
    ds.writeStrLV(smeId);
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
    //  throw protogen::framework::IncompatibleVersionException("SmeRemove");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case smeIdTag:
        {
          if(smeIdFlag)
          {
            throw protogen::framework::DuplicateFieldException("smeId");
          }
          smeId=ds.readStrLV();
          smeIdFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("SmeRemove",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!smeIdFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("smeId");
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

  static const uint32_t smeIdTag=1;

  uint32_t seqNum;

  std::string smeId;

  bool smeIdFlag;
};

}
}
}
}
}
#endif
