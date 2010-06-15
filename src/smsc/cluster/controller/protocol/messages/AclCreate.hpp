#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ACLCREATE_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ACLCREATE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "AclCacheType.hpp"


#ident "@(#) AclCreate version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


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
    addresses.clear();
  }
 
  static int32_t getTag()
  {
    return 28;
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
      rv+=AclCacheType::getNameByValue(cacheType);
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
      for(std::vector<std::string>::const_iterator it=addresses.begin(),end=addresses.end();it!=end;it++)
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
  int32_t length()const
  {
    int32_t rv=0;
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
  const std::vector<std::string>& getAddresses()const
  {
    if(!addressesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("addresses");
    }
    return addresses;
  }
  void setAddresses(const std::vector<std::string>& argValue)
  {
    addresses=argValue;
    addressesFlag=true;
  }
  std::vector<std::string>& getAddressesRef()
  {
    addressesFlag=true;
    return addresses;
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
    if(!addressesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("addresses");
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
    ds.writeLength(DataStream::fieldSize(addresses));
    for(std::vector<std::string>::const_iterator it=addresses.begin(),end=addresses.end();it!=end;it++)
    {
      ds.writeStr(*it);
    }
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
    //  throw protogen::framework::IncompatibleVersionException("AclCreate");
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
        case addressesTag:
        {
          if(addressesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("addresses");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            addresses.push_back(ds.readStr());
            rd+=DataStream::fieldSize(addresses.back());
          }
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
    if(!addressesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("addresses");
    }

  }

  int32_t getSeqNum()const
  {
    return seqNum;
  }

  void setSeqNum(int32_t argValue)
  {
    seqNum=argValue;
  }

 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t nameTag=1;
  static const int32_t descriptionTag=2;
  static const int32_t cacheTypeTag=3;
  static const int32_t addressesTag=4;

  int32_t seqNum;

  std::string name;
  std::string description;
  AclCacheType::type cacheType;
  std::vector<std::string> addresses;

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
