#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_ACLUPDATE_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_ACLUPDATE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "AclInfo.hpp"


#ident "@(#) AclUpdate version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


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
 
  static int32_t messageGetTag()
  {
    return 29;
  }

  static std::string messageGetName()
  {
    return "AclUpdate";
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
  int32_t length()const
  {
    int32_t rv=0;
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
      throw eyeline::protogen::framework::FieldIsNullException("acl");
    }
    return acl;
  }
  void setAcl(const AclInfo& argValue)
  {
    acl=argValue;
    aclFlag=true;
  }
  AclInfo& getAclRef()
  {
    aclFlag=true;
    return acl;
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("acl");
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
    //int8_t rdVersionMajor=ds.readByte();
    //int8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("AclUpdate");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case aclTag:
        {
          if(aclFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("acl");
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("acl");
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

  static const int32_t aclTag=1;

  int32_t seqNum;

  AclInfo acl;

  bool aclFlag;
};

}
}
}
}
#endif
