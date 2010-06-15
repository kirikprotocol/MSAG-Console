#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_DISCONNECTSERVICE_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_DISCONNECTSERVICE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) DisconnectService version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


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
 
  static int32_t getTag()
  {
    return 102;
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
  int32_t length()const
  {
    int32_t rv=0;
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
      throw eyeline::protogen::framework::FieldIsNullException("serviceId");
    }
    return serviceId;
  }
  void setServiceId(const std::string& argValue)
  {
    serviceId=argValue;
    serviceIdFlag=true;
  }
  std::string& getServiceIdRef()
  {
    serviceIdFlag=true;
    return serviceId;
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("serviceId");
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
    //int8_t rdVersionMajor=ds.readByte();
    //int8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("DisconnectService");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case serviceIdTag:
        {
          if(serviceIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("serviceId");
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("serviceId");
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

  static const int32_t serviceIdTag=1;

  int32_t seqNum;

  std::string serviceId;

  bool serviceIdFlag;
};

}
}
}
}
#endif
