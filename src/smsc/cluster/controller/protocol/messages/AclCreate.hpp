#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ACLCREATE_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ACLCREATE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) AclCreate version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class AclCreate{
public:
  AclCreate()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    nameFlag=false;
    descriptionFlag=false;
    cacheTypeFlag=false;
    addressesFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
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
    if(addressesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="addresses=";
      rv+="[";
      bool first=true;
      for(string_list::const_iterator it=addresses.begin(),end=addresses.end();it!=end;it++)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=*it;
      }
      rv+="]";
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
    if(addressesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(addresses);
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
  const string_list& getAddresses()const
  {
    if(!addressesFlag)
    {
      throw protogen::framework::FieldIsNullException("addresses");
    }
    return addresses;
  }
  void setAddresses(const string_list& value)
  {
    addresses=value;
    addressesFlag=true;
  }
  bool hasAddresses()const
  {
    return addressesFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
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
    if(!addressesFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("addresses");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(nameTag);
    ds.writeStrLV(name);
    ds.writeTag(descriptionTag);
    ds.writeStrLV(description);
    ds.writeTag(cacheTypeTag);
    ds.writeByteLV(cacheType);
    ds.writeTag(addressesTag);
    ds.writeStrLstLV(addresses);
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
    //  throw protogen::framework::IncompatibleVersionException("AclCreate");
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
        case addressesTag:
        {
          if(addressesFlag)
          {
            throw protogen::framework::DuplicateFieldException("addresses");
          }
          ds.readStrLstLV(addresses);
          addressesFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("AclCreate",tag);
          //}
          ds.skip(ds.readLength());
      }
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
    if(!addressesFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("addresses");
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

  static const uint32_t nameTag=1;
  static const uint32_t descriptionTag=2;
  static const uint32_t cacheTypeTag=3;
  static const uint32_t addressesTag=4;

  uint32_t seqNum;

  std::string name;
  std::string description;
  uint8_t cacheType;
  string_list addresses;

  bool nameFlag;
  bool descriptionFlag;
  bool cacheTypeFlag;
  bool addressesFlag;
};

}
}
}
}
}
#endif
