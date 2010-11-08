#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYSTATISTICS_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYSTATISTICS_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) DeliveryStatistics version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class DeliveryStatistics{
public:
  DeliveryStatistics()
  {
    Clear();
  }
  void Clear()
  {
    newMessagesFlag=false;
    processMessageFlag=false;
    deliveredMessagesFlag=false;
    failedMessageFlag=false;
    expiredMessagesFlag=false;
  }
 

  static std::string messageGetName()
  {
    return "DeliveryStatistics";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(newMessagesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="newMessages=";
      sprintf(buf,"%lld",newMessages);
      rv+=buf;
    }
    if(processMessageFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="processMessage=";
      sprintf(buf,"%lld",processMessage);
      rv+=buf;
    }
    if(deliveredMessagesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="deliveredMessages=";
      sprintf(buf,"%lld",deliveredMessages);
      rv+=buf;
    }
    if(failedMessageFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="failedMessage=";
      sprintf(buf,"%lld",failedMessage);
      rv+=buf;
    }
    if(expiredMessagesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="expiredMessages=";
      sprintf(buf,"%lld",expiredMessages);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(newMessagesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(newMessages);
    }
    if(processMessageFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(processMessage);
    }
    if(deliveredMessagesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(deliveredMessages);
    }
    if(failedMessageFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(failedMessage);
    }
    if(expiredMessagesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(expiredMessages);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int64_t getNewMessages()const
  {
    if(!newMessagesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("newMessages");
    }
    return newMessages;
  }
  void setNewMessages(int64_t argValue)
  {
    newMessages=argValue;
    newMessagesFlag=true;
  }
  int64_t& getNewMessagesRef()
  {
    newMessagesFlag=true;
    return newMessages;
  }
  bool hasNewMessages()const
  {
    return newMessagesFlag;
  }
  int64_t getProcessMessage()const
  {
    if(!processMessageFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("processMessage");
    }
    return processMessage;
  }
  void setProcessMessage(int64_t argValue)
  {
    processMessage=argValue;
    processMessageFlag=true;
  }
  int64_t& getProcessMessageRef()
  {
    processMessageFlag=true;
    return processMessage;
  }
  bool hasProcessMessage()const
  {
    return processMessageFlag;
  }
  int64_t getDeliveredMessages()const
  {
    if(!deliveredMessagesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("deliveredMessages");
    }
    return deliveredMessages;
  }
  void setDeliveredMessages(int64_t argValue)
  {
    deliveredMessages=argValue;
    deliveredMessagesFlag=true;
  }
  int64_t& getDeliveredMessagesRef()
  {
    deliveredMessagesFlag=true;
    return deliveredMessages;
  }
  bool hasDeliveredMessages()const
  {
    return deliveredMessagesFlag;
  }
  int64_t getFailedMessage()const
  {
    if(!failedMessageFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("failedMessage");
    }
    return failedMessage;
  }
  void setFailedMessage(int64_t argValue)
  {
    failedMessage=argValue;
    failedMessageFlag=true;
  }
  int64_t& getFailedMessageRef()
  {
    failedMessageFlag=true;
    return failedMessage;
  }
  bool hasFailedMessage()const
  {
    return failedMessageFlag;
  }
  int64_t getExpiredMessages()const
  {
    if(!expiredMessagesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("expiredMessages");
    }
    return expiredMessages;
  }
  void setExpiredMessages(int64_t argValue)
  {
    expiredMessages=argValue;
    expiredMessagesFlag=true;
  }
  int64_t& getExpiredMessagesRef()
  {
    expiredMessagesFlag=true;
    return expiredMessages;
  }
  bool hasExpiredMessages()const
  {
    return expiredMessagesFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!newMessagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("newMessages");
    }
    if(!processMessageFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("processMessage");
    }
    if(!deliveredMessagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveredMessages");
    }
    if(!failedMessageFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("failedMessage");
    }
    if(!expiredMessagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("expiredMessages");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(newMessagesTag);
    ds.writeInt64LV(newMessages); 
    ds.writeTag(processMessageTag);
    ds.writeInt64LV(processMessage); 
    ds.writeTag(deliveredMessagesTag);
    ds.writeInt64LV(deliveredMessages); 
    ds.writeTag(failedMessageTag);
    ds.writeInt64LV(failedMessage); 
    ds.writeTag(expiredMessagesTag);
    ds.writeInt64LV(expiredMessages); 
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
    //  throw protogen::framework::IncompatibleVersionException("DeliveryStatistics");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case newMessagesTag:
        {
          if(newMessagesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("newMessages");
          }
          newMessages=ds.readInt64LV();
          newMessagesFlag=true;
        }break;
        case processMessageTag:
        {
          if(processMessageFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("processMessage");
          }
          processMessage=ds.readInt64LV();
          processMessageFlag=true;
        }break;
        case deliveredMessagesTag:
        {
          if(deliveredMessagesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("deliveredMessages");
          }
          deliveredMessages=ds.readInt64LV();
          deliveredMessagesFlag=true;
        }break;
        case failedMessageTag:
        {
          if(failedMessageFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("failedMessage");
          }
          failedMessage=ds.readInt64LV();
          failedMessageFlag=true;
        }break;
        case expiredMessagesTag:
        {
          if(expiredMessagesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("expiredMessages");
          }
          expiredMessages=ds.readInt64LV();
          expiredMessagesFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DeliveryStatistics",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!newMessagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("newMessages");
    }
    if(!processMessageFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("processMessage");
    }
    if(!deliveredMessagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveredMessages");
    }
    if(!failedMessageFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("failedMessage");
    }
    if(!expiredMessagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("expiredMessages");
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

  static const int32_t newMessagesTag=1;
  static const int32_t processMessageTag=4;
  static const int32_t deliveredMessagesTag=2;
  static const int32_t failedMessageTag=3;
  static const int32_t expiredMessagesTag=5;

  int connId;

  int64_t newMessages;
  int64_t processMessage;
  int64_t deliveredMessages;
  int64_t failedMessage;
  int64_t expiredMessages;

  bool newMessagesFlag;
  bool processMessageFlag;
  bool deliveredMessagesFlag;
  bool failedMessageFlag;
  bool expiredMessagesFlag;
};

}
}
}
}
#endif
