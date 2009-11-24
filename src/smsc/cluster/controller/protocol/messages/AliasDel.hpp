#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ALIASDEL_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ALIASDEL_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) AliasDel version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class AliasDel{
public:
  AliasDel()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    aliasFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(aliasFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="alias=";
      rv+=alias;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(aliasFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(alias);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getAlias()const
  {
    if(!aliasFlag)
    {
      throw protogen::framework::FieldIsNullException("alias");
    }
    return alias;
  }
  void setAlias(const std::string& value)
  {
    alias=value;
    aliasFlag=true;
  }
  bool hasAlias()const
  {
    return aliasFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!aliasFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("alias");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(aliasTag);
    ds.writeStrLV(alias);
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
    //  throw protogen::framework::IncompatibleVersionException("AliasDel");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case aliasTag:
        {
          if(aliasFlag)
          {
            throw protogen::framework::DuplicateFieldException("alias");
          }
          alias=ds.readStrLV();
          aliasFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("AliasDel",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!aliasFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("alias");
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

  static const uint32_t aliasTag=1;

  uint32_t seqNum;

  std::string alias;

  bool aliasFlag;
};

}
}
}
}
}
#endif
