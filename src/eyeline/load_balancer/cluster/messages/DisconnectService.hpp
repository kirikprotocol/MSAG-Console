#ifndef __GENERATED_MESSAGE_EYELINE_LOAD_BALANCER_CLUSTER_MESSAGES_DISCONNECTSERVICE_HPP__
#define __GENERATED_MESSAGE_EYELINE_LOAD_BALANCER_CLUSTER_MESSAGES_DISCONNECTSERVICE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) DisconnectService version 1.0"



namespace eyeline{
namespace load_balancer{
namespace cluster{
namespace messages{

typedef std::vector<std::string> string_list;

class DisconnectService{
public:
  DisconnectService()
  {
    Clear();
  }
  void Clear()
  {
 
    seqNum=0;
 
    serviceIdFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(serviceIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="serviceId=";
      rv+=serviceId;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(serviceIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(serviceId);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getServiceId()const
  {
    if(!serviceIdFlag)
    {
      throw protogen::framework::FieldIsNullException("serviceId");
    }
    return serviceId;
  }
  void setServiceId(const std::string& value)
  {
    serviceId=value;
    serviceIdFlag=true;
  }
  bool hasServiceId()const
  {
    return serviceIdFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!serviceIdFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("serviceId");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(serviceIdTag);
    ds.writeStrLV(serviceId);
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
    //  throw protogen::framework::IncompatibleVersionException("DisconnectService");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case serviceIdTag:
        {
          if(serviceIdFlag)
          {
            throw protogen::framework::DuplicateFieldException("serviceId");
          }
          serviceId=ds.readStrLV();
          serviceIdFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DisconnectService",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!serviceIdFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("serviceId");
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

  static const uint32_t serviceIdTag=1;

  uint32_t seqNum;
 

  std::string serviceId;

  bool serviceIdFlag;
};

}
}
}
}
#endif
