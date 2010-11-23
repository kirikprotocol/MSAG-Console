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
 
  static int32_t messageGetTag()
  {
    return 33;
  }

  static std::string messageGetName()
  {
    return "CgmAddGroup";
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
      sprintf(buf,"%d",id);
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
  int32_t length()const
  {
    int32_t rv=0;
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
  int32_t getId()const
  {
    if(!idFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("id");
    }
    return id;
  }
  void setId(int32_t argValue)
  {
    id=argValue;
    idFlag=true;
  }
  int32_t& getIdRef()
  {
    idFlag=true;
    return id;
  }
  bool hasId()const
  {
    return idFlag;
  }
  const std::string& getName()const
  {
    if(!nameFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("name");
    }
    return name;
  }
  void setName(const std::string& argValue)
  {
    name=argValue;
    nameFlag=true;
  }
  std::string& getNameRef()
  {
    nameFlag=true;
    return name;
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!nameFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("name");
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
    //int8_t rdVersionMajor=ds.readByte();
    //int8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("CgmAddGroup");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case idTag:
        {
          if(idFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("id");
          }
          id=ds.readInt32LV();
          idFlag=true;
        }break;
        case nameTag:
        {
          if(nameFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("name");
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!nameFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("name");
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

  static const int32_t idTag=1;
  static const int32_t nameTag=2;

  int32_t seqNum;

  int32_t id;
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
