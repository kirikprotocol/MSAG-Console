#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLADD_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLADD_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) DlAdd version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class DlAdd{
public:
  DlAdd()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    dlNameFlag=false;
    ownerFlag=false;
    maxElementsFlag=false;
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
    if(ownerFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="owner=";
      rv+=owner;
    }
    if(maxElementsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="maxElements=";
      sprintf(buf,"%u",(unsigned int)maxElements);
      rv+=buf;
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
    if(ownerFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(owner);
    }
    if(maxElementsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(maxElements);
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
  const std::string& getOwner()const
  {
    if(!ownerFlag)
    {
      throw protogen::framework::FieldIsNullException("owner");
    }
    return owner;
  }
  void setOwner(const std::string& value)
  {
    owner=value;
    ownerFlag=true;
  }
  bool hasOwner()const
  {
    return ownerFlag;
  }
  uint32_t getMaxElements()const
  {
    if(!maxElementsFlag)
    {
      throw protogen::framework::FieldIsNullException("maxElements");
    }
    return maxElements;
  }
  void setMaxElements(uint32_t value)
  {
    maxElements=value;
    maxElementsFlag=true;
  }
  bool hasMaxElements()const
  {
    return maxElementsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!dlNameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("dlName");
    }
    if(!maxElementsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("maxElements");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(dlNameTag);
    ds.writeStrLV(dlName);
    ds.writeTag(maxElementsTag);
    ds.writeInt32LV(maxElements);
    if(ownerFlag)
    {
      ds.writeTag(ownerTag);
    ds.writeStrLV(owner);
    }
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
    //  throw protogen::framework::IncompatibleVersionException("DlAdd");
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
        case ownerTag:
        {
          if(ownerFlag)
          {
            throw protogen::framework::DuplicateFieldException("owner");
          }
          owner=ds.readStrLV();
          ownerFlag=true;
        }break;
        case maxElementsTag:
        {
          if(maxElementsFlag)
          {
            throw protogen::framework::DuplicateFieldException("maxElements");
          }
          maxElements=ds.readInt32LV();
          maxElementsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DlAdd",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!dlNameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("dlName");
    }
    if(!maxElementsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("maxElements");
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
  static const uint32_t ownerTag=2;
  static const uint32_t maxElementsTag=3;

  uint32_t seqNum;

  std::string dlName;
  std::string owner;
  uint32_t maxElements;

  bool dlNameFlag;
  bool ownerFlag;
  bool maxElementsFlag;
};

}
}
}
}
}
#endif
