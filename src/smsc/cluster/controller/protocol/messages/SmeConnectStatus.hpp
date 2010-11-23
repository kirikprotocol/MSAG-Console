#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMECONNECTSTATUS_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMECONNECTSTATUS_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "SmeStatusType.hpp"
#include "SmeBindMode.hpp"


#ident "@(#) SmeConnectStatus version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class SmeConnectStatus{
public:
  SmeConnectStatus()
  {
    Clear();
  }
  void Clear()
  {
    statusFlag=false;
    bindModeFlag=false;
    peerInFlag=false;
    peerOutFlag=false;
    nodeIdxFlag=false;
  }
 

  static std::string messageGetName()
  {
    return "SmeConnectStatus";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(statusFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="status=";
      rv+=SmeStatusType::getNameByValue(status);
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
    if(nodeIdxFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="nodeIdx=";
      sprintf(buf,"%d",(int)nodeIdx);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(statusFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(status.getValue());
 
    }
    if(bindModeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(bindMode.getValue());
 
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
    if(nodeIdxFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(nodeIdx);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const SmeStatusType& getStatus()const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("status");
    }
    return status;
  }
  void setStatus(const SmeStatusType& argValue)
  {
    status=argValue;
    statusFlag=true;
  }
  SmeStatusType& getStatusRef()
  {
    statusFlag=true;
    return status;
  }
  bool hasStatus()const
  {
    return statusFlag;
  }
  const SmeBindMode& getBindMode()const
  {
    if(!bindModeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("bindMode");
    }
    return bindMode;
  }
  void setBindMode(const SmeBindMode& argValue)
  {
    bindMode=argValue;
    bindModeFlag=true;
  }
  SmeBindMode& getBindModeRef()
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
  int8_t getNodeIdx()const
  {
    if(!nodeIdxFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("nodeIdx");
    }
    return nodeIdx;
  }
  void setNodeIdx(int8_t argValue)
  {
    nodeIdx=argValue;
    nodeIdxFlag=true;
  }
  int8_t& getNodeIdxRef()
  {
    nodeIdxFlag=true;
    return nodeIdx;
  }
  bool hasNodeIdx()const
  {
    return nodeIdxFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }
    if(!nodeIdxFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("nodeIdx");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(statusTag);
    ds.writeByteLV(status.getValue());
 
    ds.writeTag(nodeIdxTag);
    ds.writeByteLV(nodeIdx); 
    if(bindModeFlag)
    {
      ds.writeTag(bindModeTag);
    ds.writeByteLV(bindMode.getValue());
 
    }
    if(peerInFlag)
    {
      ds.writeTag(peerInTag);
    ds.writeStrLV(peerIn); 
    }
    if(peerOutFlag)
    {
      ds.writeTag(peerOutTag);
    ds.writeStrLV(peerOut); 
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
    //  throw protogen::framework::IncompatibleVersionException("SmeConnectStatus");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case statusTag:
        {
          if(statusFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("status");
          }
          status=ds.readByteLV();
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
        case nodeIdxTag:
        {
          if(nodeIdxFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("nodeIdx");
          }
          nodeIdx=ds.readByteLV();
          nodeIdxFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("SmeConnectStatus",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }
    if(!nodeIdxFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("nodeIdx");
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t statusTag=1;
  static const int32_t bindModeTag=2;
  static const int32_t peerInTag=3;
  static const int32_t peerOutTag=4;
  static const int32_t nodeIdxTag=5;


  SmeStatusType status;
  SmeBindMode bindMode;
  std::string peerIn;
  std::string peerOut;
  int8_t nodeIdx;

  bool statusFlag;
  bool bindModeFlag;
  bool peerInFlag;
  bool peerOutFlag;
  bool nodeIdxFlag;
};

}
}
}
}
}
#endif
