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
    addrs.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 31;
  }

  static std::string messageGetName()
  {
    return "AclRemoveAddresses";
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
    if(addrsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="addrs=";
      rv+="[";
      bool first=true;
      for(std::vector<std::string>::const_iterator it=addrs.begin(),end=addrs.end();it!=end;++it)
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
  const std::vector<std::string>& getAddrs()const
  {
    if(!addrsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("addrs");
    }
    return addrs;
  }
  void setAddrs(const std::vector<std::string>& argValue)
  {
    addrs=argValue;
    addrsFlag=true;
  }
  std::vector<std::string>& getAddrsRef()
  {
    addrsFlag=true;
    return addrs;
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("aclId");
    }
    if(!addrsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("addrs");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(aclIdTag);
    ds.writeInt32LV(aclId); 
    ds.writeTag(addrsTag);
    ds.writeLength(DataStream::fieldSize(addrs));
    for(std::vector<std::string>::const_iterator it=addrs.begin(),end=addrs.end();it!=end;++it)
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
    //  throw protogen::framework::IncompatibleVersionException("AclRemoveAddresses");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
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
        case addrsTag:
        {
          if(addrsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("addrs");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            addrs.push_back(ds.readStr());
            rd+=DataStream::fieldSize(addrs.back());
            rd+=DataStream::lengthTypeSize;
          }
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("aclId");
    }
    if(!addrsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("addrs");
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

  static const int32_t aclIdTag=1;
  static const int32_t addrsTag=2;

  int32_t seqNum;

  int32_t aclId;
  std::vector<std::string> addrs;

  bool aclIdFlag;
  bool addrsFlag;
};

}
}
}
}
}
#endif
