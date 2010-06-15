#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_ACLINFO_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_ACLINFO_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "AclCacheType.hpp"


#ident "@(#) AclInfo version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class AclInfo{
public:
  AclInfo()
  {
    Clear();
  }
  void Clear()
  {
    idFlag=false;
    nameFlag=false;
    descriptionFlag=false;
    cacheTypeFlag=false;
  }
 

  std::string toString()const
  {
    std::string rv;
    char buf[32];
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
    if(descriptionFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="description=";
      rv+=description;
    }
    if(cacheTypeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="cacheType=";
      rv+=AclCacheType::getNameByValue(cacheType);
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
    if(descriptionFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(description);
    }
    if(cacheTypeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(cacheType);
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
  const std::string& getDescription()const
  {
    if(!descriptionFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("description");
    }
    return description;
  }
  void setDescription(const std::string& argValue)
  {
    description=argValue;
    descriptionFlag=true;
  }
  std::string& getDescriptionRef()
  {
    descriptionFlag=true;
    return description;
  }
  bool hasDescription()const
  {
    return descriptionFlag;
  }
  const AclCacheType::type& getCacheType()const
  {
    if(!cacheTypeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("cacheType");
    }
    return cacheType;
  }
  void setCacheType(const AclCacheType::type& argValue)
  {
    if(!AclCacheType::isValidValue(argValue))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("AclCacheType",argValue);
    }
    cacheType=argValue;
    cacheTypeFlag=true;
  }
  AclCacheType::type& getCacheTypeRef()
  {
    cacheTypeFlag=true;
    return cacheType;
  }
  bool hasCacheType()const
  {
    return cacheTypeFlag;
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
    if(!descriptionFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("description");
    }
    if(!cacheTypeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("cacheType");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(idTag);
    ds.writeInt32LV(id);
    ds.writeTag(nameTag);
    ds.writeStrLV(name);
    ds.writeTag(descriptionTag);
    ds.writeStrLV(description);
    ds.writeTag(cacheTypeTag);
    ds.writeByteLV(cacheType);
 
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
    //  throw protogen::framework::IncompatibleVersionException("AclInfo");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
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
        case descriptionTag:
        {
          if(descriptionFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("description");
          }
          description=ds.readStrLV();
          descriptionFlag=true;
        }break;
        case cacheTypeTag:
        {
          if(cacheTypeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("cacheType");
          }
          cacheType=ds.readByteLV();
          cacheTypeFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("AclInfo",tag);
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
    if(!descriptionFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("description");
    }
    if(!cacheTypeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("cacheType");
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t idTag=1;
  static const int32_t nameTag=2;
  static const int32_t descriptionTag=3;
  static const int32_t cacheTypeTag=4;


  int32_t id;
  std::string name;
  std::string description;
  AclCacheType::type cacheType;

  bool idFlag;
  bool nameFlag;
  bool descriptionFlag;
  bool cacheTypeFlag;
};

}
}
}
}
#endif
