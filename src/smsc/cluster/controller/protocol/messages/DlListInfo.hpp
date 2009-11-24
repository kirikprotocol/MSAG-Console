#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLLISTINFO_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLLISTINFO_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) DlListInfo version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class DlListInfo{
public:
  DlListInfo()
  {
    Clear();
  }
  void Clear()
  {
    nameFlag=false;
    ownerFlag=false;
    maxElFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(nameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="name=";
      rv+=name;
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
    if(maxElFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="maxEl=";
      sprintf(buf,"%u",(unsigned int)maxEl);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(nameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(name);
    }
    if(ownerFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(owner);
    }
    if(maxElFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(maxEl);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getName()const
  {
    if(!nameFlag)
    {
      throw protogen::framework::FieldIsNullException("name");
    }
    return name;
  }
  void setName(const std::string& value)
  {
    name=value;
    nameFlag=true;
  }
  bool hasName()const
  {
    return nameFlag;
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
  uint32_t getMaxEl()const
  {
    if(!maxElFlag)
    {
      throw protogen::framework::FieldIsNullException("maxEl");
    }
    return maxEl;
  }
  void setMaxEl(uint32_t value)
  {
    maxEl=value;
    maxElFlag=true;
  }
  bool hasMaxEl()const
  {
    return maxElFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!nameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("name");
    }
    if(!maxElFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("maxEl");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(nameTag);
    ds.writeStrLV(name);
    ds.writeTag(maxElTag);
    ds.writeInt32LV(maxEl);
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
    //  throw protogen::framework::IncompatibleVersionException("DlListInfo");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case nameTag:
        {
          if(nameFlag)
          {
            throw protogen::framework::DuplicateFieldException("name");
          }
          name=ds.readStrLV();
          nameFlag=true;
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
        case maxElTag:
        {
          if(maxElFlag)
          {
            throw protogen::framework::DuplicateFieldException("maxEl");
          }
          maxEl=ds.readInt32LV();
          maxElFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DlListInfo",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!nameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("name");
    }
    if(!maxElFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("maxEl");
    }

  }


protected:
  //static const uint8_t versionMajor=1;
  //static const uint8_t versionMinor=0;

  static const uint32_t nameTag=1;
  static const uint32_t ownerTag=2;
  static const uint32_t maxElTag=3;


  std::string name;
  std::string owner;
  uint32_t maxEl;

  bool nameFlag;
  bool ownerFlag;
  bool maxElFlag;
};

}
}
}
}
}
#endif
