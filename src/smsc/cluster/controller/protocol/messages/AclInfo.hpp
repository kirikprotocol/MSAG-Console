#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ACLINFO_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ACLINFO_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) AclInfo version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

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
      sprintf(buf,"%u",(unsigned int)cacheType);
      rv+=buf;
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
  const std::string& getDescription()const
  {
    if(!descriptionFlag)
    {
      throw protogen::framework::FieldIsNullException("description");
    }
    return description;
  }
  void setDescription(const std::string& value)
  {
    description=value;
    descriptionFlag=true;
  }
  bool hasDescription()const
  {
    return descriptionFlag;
  }
  uint8_t getCacheType()const
  {
    if(!cacheTypeFlag)
    {
      throw protogen::framework::FieldIsNullException("cacheType");
    }
    return cacheType;
  }
  void setCacheType(uint8_t value)
  {
    cacheType=value;
    cacheTypeFlag=true;
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
      throw protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!nameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("name");
    }
    if(!descriptionFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("description");
    }
    if(!cacheTypeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("cacheType");
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
    //uint8_t rdVersionMajor=ds.readByte();
    //uint8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("AclInfo");
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
        case descriptionTag:
        {
          if(descriptionFlag)
          {
            throw protogen::framework::DuplicateFieldException("description");
          }
          description=ds.readStrLV();
          descriptionFlag=true;
        }break;
        case cacheTypeTag:
        {
          if(cacheTypeFlag)
          {
            throw protogen::framework::DuplicateFieldException("cacheType");
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
      throw protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!nameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("name");
    }
    if(!descriptionFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("description");
    }
    if(!cacheTypeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("cacheType");
    }

  }


protected:
  //static const uint8_t versionMajor=1;
  //static const uint8_t versionMinor=0;

  static const uint32_t idTag=1;
  static const uint32_t nameTag=2;
  static const uint32_t descriptionTag=3;
  static const uint32_t cacheTypeTag=4;


  uint32_t id;
  std::string name;
  std::string description;
  uint8_t cacheType;

  bool idFlag;
  bool nameFlag;
  bool descriptionFlag;
  bool cacheTypeFlag;
};

}
}
}
}
}
#endif
