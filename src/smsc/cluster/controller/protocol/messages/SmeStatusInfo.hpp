#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMESTATUSINFO_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMESTATUSINFO_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "SmeConnectType.hpp"
#include "SmeConnectStatus.hpp"


#ident "@(#) SmeStatusInfo version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
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
    connTypeFlag=false;
    statusFlag=false;
    status.clear();
  }
 

  static std::string messageGetName()
  {
    return "SmeStatusInfo";
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
    if(connTypeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="connType=";
      rv+=SmeConnectType::getNameByValue(connType);
    }
    if(statusFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="status=";
      rv+="[";
      bool first=true;
      for(std::vector<SmeConnectStatus>::const_iterator it=status.begin(),end=status.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+="(";
        rv+=it->toString();
        rv+=")";
      }
      rv+="]";
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
    if(connTypeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(connType.getValue());
 
    }
    if(statusFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      for(std::vector<SmeConnectStatus>::const_iterator it=status.begin(),end=status.end();it!=end;++it)
      {
        rv+=it->length<DataStream>();
      }
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
  const SmeConnectType& getConnType()const
  {
    if(!connTypeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("connType");
    }
    return connType;
  }
  void setConnType(const SmeConnectType& argValue)
  {
    connType=argValue;
    connTypeFlag=true;
  }
  SmeConnectType& getConnTypeRef()
  {
    connTypeFlag=true;
    return connType;
  }
  bool hasConnType()const
  {
    return connTypeFlag;
  }
  const std::vector<SmeConnectStatus>& getStatus()const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("status");
    }
    return status;
  }
  void setStatus(const std::vector<SmeConnectStatus>& argValue)
  {
    status=argValue;
    statusFlag=true;
  }
  std::vector<SmeConnectStatus>& getStatusRef()
  {
    statusFlag=true;
    return status;
  }
  bool hasStatus()const
  {
    return statusFlag;
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
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(systemIdTag);
    ds.writeStrLV(systemId); 
    ds.writeTag(statusTag);
    typename DataStream::LengthType len=0;
    for(std::vector<SmeConnectStatus>::const_iterator it=status.begin(),end=status.end();it!=end;++it)
    {
      len+=it->length<DataStream>();
    }
    ds.writeLength(len);
    for(std::vector<SmeConnectStatus>::const_iterator it=status.begin(),end=status.end();it!=end;++it)
    {
      it->serialize(ds);
    }
    if(connTypeFlag)
    {
      ds.writeTag(connTypeTag);
    ds.writeByteLV(connType.getValue());
 
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
    //  throw protogen::framework::IncompatibleVersionException("SmeStatusInfo");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
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
        case connTypeTag:
        {
          if(connTypeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("connType");
          }
          connType=ds.readByteLV();
          connTypeFlag=true;
        }break;
        case statusTag:
        {
          if(statusFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("status");
          }

          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            status.push_back(SmeConnectStatus());
            status.back().deserialize(ds);
            rd+=status.back().length<DataStream>();
          }
          statusFlag=true;
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

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t systemIdTag=1;
  static const int32_t connTypeTag=2;
  static const int32_t statusTag=3;


  std::string systemId;
  SmeConnectType connType;
  std::vector<SmeConnectStatus> status;

  bool systemIdFlag;
  bool connTypeFlag;
  bool statusFlag;
};

}
}
}
}
}
#endif
