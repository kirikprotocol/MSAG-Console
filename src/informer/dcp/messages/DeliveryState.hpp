#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYSTATE_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYSTATE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryStatus.hpp"


#ident "@(#) DeliveryState version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class DeliveryState{
public:
  DeliveryState()
  {
    Clear();
  }
  void Clear()
  {
    statusFlag=false;
    dateFlag=false;
  }
 

  static std::string messageGetName()
  {
    return "DeliveryState";
  }

  std::string toString()const
  {
    std::string rv;
    if(statusFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="status=";
      rv+=DeliveryStatus::getNameByValue(status);
    }
    if(dateFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="date=";
      rv+=date;
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
    if(dateFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(date);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const DeliveryStatus& getStatus()const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("status");
    }
    return status;
  }
  void setStatus(const DeliveryStatus& argValue)
  {
    status=argValue;
    statusFlag=true;
  }
  DeliveryStatus& getStatusRef()
  {
    statusFlag=true;
    return status;
  }
  bool hasStatus()const
  {
    return statusFlag;
  }
  const std::string& getDate()const
  {
    if(!dateFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("date");
    }
    return date;
  }
  void setDate(const std::string& argValue)
  {
    date=argValue;
    dateFlag=true;
  }
  std::string& getDateRef()
  {
    dateFlag=true;
    return date;
  }
  bool hasDate()const
  {
    return dateFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(statusTag);
    ds.writeByteLV(status.getValue());
 
    if(dateFlag)
    {
      ds.writeTag(dateTag);
    ds.writeStrLV(date); 
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
    //  throw protogen::framework::IncompatibleVersionException("DeliveryState");
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
        case dateTag:
        {
          if(dateFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("date");
          }
          date=ds.readStrLV();
          dateFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DeliveryState",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }

  }


  void messageSetConnId(int argConnId)
  {
    connId=argConnId;
  }

  int messageGetConnId()const
  {
    return connId;
  }
 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t statusTag=1;
  static const int32_t dateTag=2;

  int connId;

  DeliveryStatus status;
  std::string date;

  bool statusFlag;
  bool dateFlag;
};

}
}
}
}
#endif
