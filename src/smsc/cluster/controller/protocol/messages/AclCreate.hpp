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


class AclCreate{
public:
  AclCreate()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    idFlag=false;
    nameFlag=false;
    descriptionFlag=false;
    addressesFlag=false;
    addresses.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 28;
  }

  static std::string messageGetName()
  {
    return "AclCreate";
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
    if(descriptionFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="description=";
      rv+=description;
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
      for(std::vector<std::string>::const_iterator it=addresses.begin(),end=addresses.end();it!=end;++it)
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
    if(addressesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(addresses);
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
    ds.writeTag(addressesTag);
    ds.writeLength(DataStream::fieldSize(addresses));
    for(std::vector<std::string>::const_iterator it=addresses.begin(),end=addresses.end();it!=end;++it)
    {
      ds.writeStr(*it);
          }
    if(idFlag)
    {
      ds.writeTag(idTag);
    ds.writeInt32LV(id); 
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
        case descriptionTag:
        {
          if(descriptionFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("description");
          }
          description=ds.readStrLV();
          descriptionFlag=true;
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
            rd+=DataStream::lengthTypeSize;
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
    if(!addressesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("addresses");
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

  static const int32_t idTag=5;
  static const int32_t nameTag=1;
  static const int32_t descriptionTag=2;
  static const int32_t addressesTag=4;

  int32_t seqNum;

  int32_t id;
  std::string name;
  std::string description;
  std::vector<std::string> addresses;

  bool idFlag;
  bool nameFlag;
  bool descriptionFlag;
  bool addressesFlag;
};

}
}
}
}
}
#endif
