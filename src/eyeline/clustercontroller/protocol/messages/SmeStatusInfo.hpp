#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMESTATUSINFO_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMESTATUSINFO_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "SmeBindMode.hpp"


#ident "@(#) SmeStatusInfo version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class SmeStatusInfo{
public:
  SmeStatusInfo()
  {
    Clear();
  }
  void Clear()
  {
    systemIdFlag=false;
    statusFlag=false;
    bindModeFlag=false;
    peerInFlag=false;
    peerOutFlag=false;
  }
 

  std::string toString()const
  {
    std::string rv;
    if(systemIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="systemId=";
      rv+=systemId;
    }
    if(statusFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="status=";
      rv+=status;
    }
    if(bindModeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="bindMode=";
      rv+=SmeBindMode::getNameByValue(bindMode);
    }
    if(peerInFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="peerIn=";
      rv+=peerIn;
    }
    if(peerOutFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="peerOut=";
      rv+=peerOut;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(systemIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(systemId);
    }
    if(statusFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(status);
    }
    if(bindModeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(bindMode);
    }
    if(peerInFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(peerIn);
    }
    if(peerOutFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(peerOut);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getSystemId()const
  {
    if(!systemIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("systemId");
    }
    return systemId;
  }
  void setSystemId(const std::string& argValue)
  {
    systemId=argValue;
    systemIdFlag=true;
  }
  std::string& getSystemIdRef()
  {
    systemIdFlag=true;
    return systemId;
  }
  bool hasSystemId()const
  {
    return systemIdFlag;
  }
  const std::string& getStatus()const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("status");
    }
    return status;
  }
  void setStatus(const std::string& argValue)
  {
    status=argValue;
    statusFlag=true;
  }
  std::string& getStatusRef()
  {
    statusFlag=true;
    return status;
  }
  bool hasStatus()const
  {
    return statusFlag;
  }
  const SmeBindMode::type& getBindMode()const
  {
    if(!bindModeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("bindMode");
    }
    return bindMode;
  }
  void setBindMode(const SmeBindMode::type& argValue)
  {
    if(!SmeBindMode::isValidValue(argValue))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("SmeBindMode",argValue);
    }
    bindMode=argValue;
    bindModeFlag=true;
  }
  SmeBindMode::type& getBindModeRef()
  {
    bindModeFlag=true;
    return bindMode;
  }
  bool hasBindMode()const
  {
    return bindModeFlag;
  }
  const std::string& getPeerIn()const
  {
    if(!peerInFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("peerIn");
    }
    return peerIn;
  }
  void setPeerIn(const std::string& argValue)
  {
    peerIn=argValue;
    peerInFlag=true;
  }
  std::string& getPeerInRef()
  {
    peerInFlag=true;
    return peerIn;
  }
  bool hasPeerIn()const
  {
    return peerInFlag;
  }
  const std::string& getPeerOut()const
  {
    if(!peerOutFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("peerOut");
    }
    return peerOut;
  }
  void setPeerOut(const std::string& argValue)
  {
    peerOut=argValue;
    peerOutFlag=true;
  }
  std::string& getPeerOutRef()
  {
    peerOutFlag=true;
    return peerOut;
  }
  bool hasPeerOut()const
  {
    return peerOutFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!systemIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("systemId");
    }
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }
    if(!bindModeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("bindMode");
    }
    if(!peerInFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("peerIn");
    }
    if(!peerOutFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("peerOut");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(systemIdTag);
    ds.writeStrLV(systemId);
    ds.writeTag(statusTag);
    ds.writeStrLV(status);
    ds.writeTag(bindModeTag);
    ds.writeByteLV(bindMode);
 
    ds.writeTag(peerInTag);
    ds.writeStrLV(peerIn);
    ds.writeTag(peerOutTag);
    ds.writeStrLV(peerOut);
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
    //  throw protogen::framework::IncompatibleVersionException("SmeStatusInfo");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case systemIdTag:
        {
          if(systemIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("systemId");
          }
          systemId=ds.readStrLV();
          systemIdFlag=true;
        }break;
        case statusTag:
        {
          if(statusFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("status");
          }
          status=ds.readStrLV();
          statusFlag=true;
        }break;
        case bindModeTag:
        {
          if(bindModeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("bindMode");
          }
          bindMode=ds.readByteLV();
          bindModeFlag=true;
        }break;
        case peerInTag:
        {
          if(peerInFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("peerIn");
          }
          peerIn=ds.readStrLV();
          peerInFlag=true;
        }break;
        case peerOutTag:
        {
          if(peerOutFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("peerOut");
          }
          peerOut=ds.readStrLV();
          peerOutFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("SmeStatusInfo",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!systemIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("systemId");
    }
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }
    if(!bindModeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("bindMode");
    }
    if(!peerInFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("peerIn");
    }
    if(!peerOutFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("peerOut");
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t systemIdTag=1;
  static const int32_t statusTag=2;
  static const int32_t bindModeTag=3;
  static const int32_t peerInTag=4;
  static const int32_t peerOutTag=5;


  std::string systemId;
  std::string status;
  SmeBindMode::type bindMode;
  std::string peerIn;
  std::string peerOut;

  bool systemIdFlag;
  bool statusFlag;
  bool bindModeFlag;
  bool peerInFlag;
  bool peerOutFlag;
};

}
}
}
}
#endif
