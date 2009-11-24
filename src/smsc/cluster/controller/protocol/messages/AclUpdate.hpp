#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ACLUPDATE_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ACLUPDATE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "AclInfo.hpp"


#ident "@(#) AclUpdate version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class AclUpdate{
public:
  AclUpdate()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    aclFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(aclFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="acl=";
      rv+='(';
      rv+=acl.toString();
      rv+=')';
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(aclFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=acl.length<DataStream>();
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const AclInfo& getAcl()const
  {
    if(!aclFlag)
    {
      throw protogen::framework::FieldIsNullException("acl");
    }
    return acl;
  }
  void setAcl(const AclInfo& value)
  {
    acl=value;
    aclFlag=true;
  }
  bool hasAcl()const
  {
    return aclFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!aclFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("acl");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(aclTag);
    ds.writeLength(acl.length<DataStream>());
    acl.serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("AclUpdate");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case aclTag:
        {
          if(aclFlag)
          {
            throw protogen::framework::DuplicateFieldException("acl");
          }

          ds.readLength();acl.deserialize(ds);
          aclFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("AclUpdate",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!aclFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("acl");
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

  static const uint32_t aclTag=1;

  uint32_t seqNum;

  AclInfo acl;

  bool aclFlag;
};

}
}
}
}
}
#endif
