#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_ADDDELIVERYMESSAGES_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_ADDDELIVERYMESSAGES_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryMessage.hpp"


#ident "@(#) AddDeliveryMessages version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class AddDeliveryMessages{
public:
  AddDeliveryMessages()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    deliveryIdFlag=false;
    messagesFlag=false;
    messages.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 35;
  }

  static std::string messageGetName()
  {
    return "AddDeliveryMessages";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(deliveryIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="deliveryId=";
      sprintf(buf,"%d",deliveryId);
      rv+=buf;
    }
    if(messagesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="messages=";
      rv+="[";
      bool first=true;
      for(std::vector<DeliveryMessage>::const_iterator it=messages.begin(),end=messages.end();it!=end;++it)
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
    if(deliveryIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(deliveryId);
    }
    if(messagesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      for(std::vector<DeliveryMessage>::const_iterator it=messages.begin(),end=messages.end();it!=end;++it)
      {
        rv+=it->length<DataStream>();
      }
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getDeliveryId()const
  {
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("deliveryId");
    }
    return deliveryId;
  }
  void setDeliveryId(int32_t argValue)
  {
    deliveryId=argValue;
    deliveryIdFlag=true;
  }
  int32_t& getDeliveryIdRef()
  {
    deliveryIdFlag=true;
    return deliveryId;
  }
  bool hasDeliveryId()const
  {
    return deliveryIdFlag;
  }
  const std::vector<DeliveryMessage>& getMessages()const
  {
    if(!messagesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("messages");
    }
    return messages;
  }
  void setMessages(const std::vector<DeliveryMessage>& argValue)
  {
    messages=argValue;
    messagesFlag=true;
  }
  std::vector<DeliveryMessage>& getMessagesRef()
  {
    messagesFlag=true;
    return messages;
  }
  bool hasMessages()const
  {
    return messagesFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryId");
    }
    if(!messagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("messages");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(deliveryIdTag);
    ds.writeInt32LV(deliveryId); 
    ds.writeTag(messagesTag);
    typename DataStream::LengthType len=0;
    for(std::vector<DeliveryMessage>::const_iterator it=messages.begin(),end=messages.end();it!=end;++it)
    {
      len+=it->length<DataStream>();
    }
    ds.writeLength(len);
    for(std::vector<DeliveryMessage>::const_iterator it=messages.begin(),end=messages.end();it!=end;++it)
    {
      it->serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("AddDeliveryMessages");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case deliveryIdTag:
        {
          if(deliveryIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("deliveryId");
          }
          deliveryId=ds.readInt32LV();
          deliveryIdFlag=true;
        }break;
        case messagesTag:
        {
          if(messagesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("messages");
          }

          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            messages.push_back(DeliveryMessage());
            messages.back().deserialize(ds);
            rd+=messages.back().length<DataStream>();
          }
          messagesFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("AddDeliveryMessages",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryId");
    }
    if(!messagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("messages");
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

  static const int32_t deliveryIdTag=1;
  static const int32_t messagesTag=2;

  int32_t seqNum;
  int connId;

  int32_t deliveryId;
  std::vector<DeliveryMessage> messages;

  bool deliveryIdFlag;
  bool messagesFlag;
};

}
}
}
}
#endif
