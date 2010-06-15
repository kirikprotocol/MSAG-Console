#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_CATEGORYINFO_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_CATEGORYINFO_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) CategoryInfo version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class CategoryInfo{
public:
  CategoryInfo()
  {
    Clear();
  }
  void Clear()
  {
    nameFlag=false;
    levelFlag=false;
  }
 

  std::string toString()const
  {
    std::string rv;
    if(nameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="name=";
      rv+=name;
    }
    if(levelFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="level=";
      rv+=level;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(nameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(name);
    }
    if(levelFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(level);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
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
  const std::string& getLevel()const
  {
    if(!levelFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("level");
    }
    return level;
  }
  void setLevel(const std::string& argValue)
  {
    level=argValue;
    levelFlag=true;
  }
  std::string& getLevelRef()
  {
    levelFlag=true;
    return level;
  }
  bool hasLevel()const
  {
    return levelFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!nameFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("name");
    }
    if(!levelFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("level");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(nameTag);
    ds.writeStrLV(name);
    ds.writeTag(levelTag);
    ds.writeStrLV(level);
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
    //  throw protogen::framework::IncompatibleVersionException("CategoryInfo");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case nameTag:
        {
          if(nameFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("name");
          }
          name=ds.readStrLV();
          nameFlag=true;
        }break;
        case levelTag:
        {
          if(levelFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("level");
          }
          level=ds.readStrLV();
          levelFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("CategoryInfo",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!nameFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("name");
    }
    if(!levelFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("level");
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t nameTag=1;
  static const int32_t levelTag=2;


  std::string name;
  std::string level;

  bool nameFlag;
  bool levelFlag;
};

}
}
}
}
}
#endif
