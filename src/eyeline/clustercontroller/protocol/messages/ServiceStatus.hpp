#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SERVICESTATUS_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SERVICESTATUS_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) ServiceStatus version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class ServiceStatus{
public:
  ServiceStatus()
  {
    Clear();
  }
  void Clear()
  {
    serviceNameFlag=false;
    peerAddressFlag=false;
    bindModeFlag=false;
    boundSmscFlag=false;
    boundSmsc.clear();
  }
 

  std::string toString()const
  {
    std::string rv;
    if(serviceNameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="serviceName=";
      rv+=serviceName;
    }
    if(peerAddressFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="peerAddress=";
      rv+=peerAddress;
    }
    if(bindModeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="bindMode=";
      rv+=bindMode;
    }
    if(boundSmscFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="boundSmsc=";
      rv+="[";
      bool first=true;
      for(std::vector<bool>::const_iterator it=boundSmsc.begin(),end=boundSmsc.end();it!=end;it++)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=*it?"true":"false";
      }
      rv+="]";
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(serviceNameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(serviceName);
    }
    if(peerAddressFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(peerAddress);
    }
    if(bindModeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(bindMode);
    }
    if(boundSmscFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(boundSmsc);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getServiceName()const
  {
    if(!serviceNameFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("serviceName");
    }
    return serviceName;
  }
  void setServiceName(const std::string& argValue)
  {
    serviceName=argValue;
    serviceNameFlag=true;
  }
  std::string& getServiceNameRef()
  {
    serviceNameFlag=true;
    return serviceName;
  }
  bool hasServiceName()const
  {
    return serviceNameFlag;
  }
  const std::string& getPeerAddress()const
  {
    if(!peerAddressFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("peerAddress");
    }
    return peerAddress;
  }
  void setPeerAddress(const std::string& argValue)
  {
    peerAddress=argValue;
    peerAddressFlag=true;
  }
  std::string& getPeerAddressRef()
  {
    peerAddressFlag=true;
    return peerAddress;
  }
  bool hasPeerAddress()const
  {
    return peerAddressFlag;
  }
  const std::string& getBindMode()const
  {
    if(!bindModeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("bindMode");
    }
    return bindMode;
  }
  void setBindMode(const std::string& argValue)
  {
    bindMode=argValue;
    bindModeFlag=true;
  }
  std::string& getBindModeRef()
  {
    bindModeFlag=true;
    return bindMode;
  }
  bool hasBindMode()const
  {
    return bindModeFlag;
  }
  const std::vector<bool>& getBoundSmsc()const
  {
    if(!boundSmscFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("boundSmsc");
    }
    return boundSmsc;
  }
  void setBoundSmsc(const std::vector<bool>& argValue)
  {
    boundSmsc=argValue;
    boundSmscFlag=true;
  }
  std::vector<bool>& getBoundSmscRef()
  {
    boundSmscFlag=true;
    return boundSmsc;
  }
  bool hasBoundSmsc()const
  {
    return boundSmscFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!serviceNameFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("serviceName");
    }
    if(!peerAddressFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("peerAddress");
    }
    if(!bindModeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("bindMode");
    }
    if(!boundSmscFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("boundSmsc");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(serviceNameTag);
    ds.writeStrLV(serviceName);
    ds.writeTag(peerAddressTag);
    ds.writeStrLV(peerAddress);
    ds.writeTag(bindModeTag);
    ds.writeStrLV(bindMode);
    ds.writeTag(boundSmscTag);
    ds.writeLength(DataStream::fieldSize(boundSmsc));
    for(std::vector<bool>::const_iterator it=boundSmsc.begin(),end=boundSmsc.end();it!=end;it++)
    {
      ds.writeBool(*it);
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
    //  throw protogen::framework::IncompatibleVersionException("ServiceStatus");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case serviceNameTag:
        {
          if(serviceNameFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("serviceName");
          }
          serviceName=ds.readStrLV();
          serviceNameFlag=true;
        }break;
        case peerAddressTag:
        {
          if(peerAddressFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("peerAddress");
          }
          peerAddress=ds.readStrLV();
          peerAddressFlag=true;
        }break;
        case bindModeTag:
        {
          if(bindModeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("bindMode");
          }
          bindMode=ds.readStrLV();
          bindModeFlag=true;
        }break;
        case boundSmscTag:
        {
          if(boundSmscFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("boundSmsc");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            boundSmsc.push_back(ds.readBool());
            rd+=DataStream::fieldSize(boundSmsc.back());
          }
          boundSmscFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("ServiceStatus",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!serviceNameFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("serviceName");
    }
    if(!peerAddressFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("peerAddress");
    }
    if(!bindModeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("bindMode");
    }
    if(!boundSmscFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("boundSmsc");
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t serviceNameTag=1;
  static const int32_t peerAddressTag=2;
  static const int32_t bindModeTag=3;
  static const int32_t boundSmscTag=4;


  std::string serviceName;
  std::string peerAddress;
  std::string bindMode;
  std::vector<bool> boundSmsc;

  bool serviceNameFlag;
  bool peerAddressFlag;
  bool bindModeFlag;
  bool boundSmscFlag;
};

}
}
}
}
#endif
