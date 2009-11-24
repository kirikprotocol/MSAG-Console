#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_CGMADDGROUP_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_CGMADDGROUP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) CgmAddGroup version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class CgmAddGroup{
public:
  CgmAddGroup()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    idFlag=false;
    nameFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(idFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="id=";
      sprintf(buf,"%u",(unsigned int)id);
      rv+=buf;
    }
    if(nameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="name=";
      rv+=name;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(idFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(id);
    }
    if(nameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(name);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  uint32_t getId()const
  {
    if(!idFlag)
    {
      throw protogen::framework::FieldIsNullException("id");
    }
    return id;
  }
  void setId(uint32_t value)
  {
    id=value;
    idFlag=true;
  }
  bool hasId()const
  {
    return idFlag;
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
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!idFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!nameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("name");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(idTag);
    ds.writeInt32LV(id);
    ds.writeTag(nameTag);
    ds.writeStrLV(name);
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
    //  throw protogen::framework::IncompatibleVersionException("CgmAddGroup");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case idTag:
        {
          if(idFlag)
          {
            throw protogen::framework::DuplicateFieldException("id");
          }
          id=ds.readInt32LV();
          idFlag=true;
        }break;
        case nameTag:
        {
          if(nameFlag)
          {
            throw protogen::framework::DuplicateFieldException("name");
          }
          name=ds.readStrLV();
          nameFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("CgmAddGroup",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!idFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!nameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("name");
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

  static const uint32_t idTag=1;
  static const uint32_t nameTag=2;

  uint32_t seqNum;

  uint32_t id;
  std::string name;

  bool idFlag;
  bool nameFlag;
};

}
}
}
}
}
#endif
