#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ACLREMOVEADDRESSES_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ACLREMOVEADDRESSES_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) AclRemoveAddresses version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class AclRemoveAddresses{
public:
  AclRemoveAddresses()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    aclIdFlag=false;
    addrsFlag=false;
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
      sprintf(buf,"%u",(unsigned int)aclId);
      rv+=buf;
    }
    if(addrsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="addrs=";
      rv+="[";
      bool first=true;
      for(string_list::const_iterator it=addrs.begin(),end=addrs.end();it!=end;it++)
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
    if(aclIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(aclId);
    }
    if(addrsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(addrs);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  uint32_t getAclId()const
  {
    if(!aclIdFlag)
    {
      throw protogen::framework::FieldIsNullException("aclId");
    }
    return aclId;
  }
  void setAclId(uint32_t value)
  {
    aclId=value;
    aclIdFlag=true;
  }
  bool hasAclId()const
  {
    return aclIdFlag;
  }
  const string_list& getAddrs()const
  {
    if(!addrsFlag)
    {
      throw protogen::framework::FieldIsNullException("addrs");
    }
    return addrs;
  }
  void setAddrs(const string_list& value)
  {
    addrs=value;
    addrsFlag=true;
  }
  bool hasAddrs()const
  {
    return addrsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!aclIdFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("aclId");
    }
    if(!addrsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("addrs");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(aclIdTag);
    ds.writeInt32LV(aclId);
    ds.writeTag(addrsTag);
    ds.writeStrLstLV(addrs);
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
    //  throw protogen::framework::IncompatibleVersionException("AclRemoveAddresses");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case aclIdTag:
        {
          if(aclIdFlag)
          {
            throw protogen::framework::DuplicateFieldException("aclId");
          }
          aclId=ds.readInt32LV();
          aclIdFlag=true;
        }break;
        case addrsTag:
        {
          if(addrsFlag)
          {
            throw protogen::framework::DuplicateFieldException("addrs");
          }
          ds.readStrLstLV(addrs);
          addrsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("AclRemoveAddresses",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!aclIdFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("aclId");
    }
    if(!addrsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("addrs");
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

  static const uint32_t aclIdTag=1;
  static const uint32_t addrsTag=2;

  uint32_t seqNum;

  uint32_t aclId;
  string_list addrs;

  bool aclIdFlag;
  bool addrsFlag;
};

}
}
}
}
}
#endif
