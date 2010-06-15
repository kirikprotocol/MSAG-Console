#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_ACLLOOKUP_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_ACLLOOKUP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) AclLookup version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class AclLookup{
public:
  AclLookup()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    aclIdFlag=false;
    addrPrefixFlag=false;
  }
 
  static int32_t getTag()
  {
    return 30;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(aclIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="aclId=";
      sprintf(buf,"%d",aclId);
      rv+=buf;
    }
    if(addrPrefixFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="addrPrefix=";
      rv+=addrPrefix;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(aclIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(aclId);
    }
    if(addrPrefixFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(addrPrefix);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getAclId()const
  {
    if(!aclIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("aclId");
    }
    return aclId;
  }
  void setAclId(int32_t argValue)
  {
    aclId=argValue;
    aclIdFlag=true;
  }
  int32_t& getAclIdRef()
  {
    aclIdFlag=true;
    return aclId;
  }
  bool hasAclId()const
  {
    return aclIdFlag;
  }
  const std::string& getAddrPrefix()const
  {
    if(!addrPrefixFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("addrPrefix");
    }
    return addrPrefix;
  }
  void setAddrPrefix(const std::string& argValue)
  {
    addrPrefix=argValue;
    addrPrefixFlag=true;
  }
  std::string& getAddrPrefixRef()
  {
    addrPrefixFlag=true;
    return addrPrefix;
  }
  bool hasAddrPrefix()const
  {
    return addrPrefixFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!aclIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("aclId");
    }
    if(!addrPrefixFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("addrPrefix");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(aclIdTag);
    ds.writeInt32LV(aclId);
    ds.writeTag(addrPrefixTag);
    ds.writeStrLV(addrPrefix);
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
    //  throw protogen::framework::IncompatibleVersionException("AclLookup");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case aclIdTag:
        {
          if(aclIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("aclId");
          }
          aclId=ds.readInt32LV();
          aclIdFlag=true;
        }break;
        case addrPrefixTag:
        {
          if(addrPrefixFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("addrPrefix");
          }
          addrPrefix=ds.readStrLV();
          addrPrefixFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("AclLookup",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!aclIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("aclId");
    }
    if(!addrPrefixFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("addrPrefix");
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

  static const int32_t aclIdTag=1;
  static const int32_t addrPrefixTag=2;

  int32_t seqNum;

  int32_t aclId;
  std::string addrPrefix;

  bool aclIdFlag;
  bool addrPrefixFlag;
};

}
}
}
}
#endif
