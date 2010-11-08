#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DROPDELIVERYMESSAGES_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DROPDELIVERYMESSAGES_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) DropDeliveryMessages version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class DropDeliveryMessages{
public:
  DropDeliveryMessages()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    deliveryIdFlag=false;
    messageIdsFlag=false;
    messageIds.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 37;
  }

  static std::string messageGetName()
  {
    return "DropDeliveryMessages";
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
    if(messageIdsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="messageIds=";
      rv+="[";
      bool first=true;
      for(std::vector<int64_t>::const_iterator it=messageIds.begin(),end=messageIds.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        sprintf(buf,"%lld",*it);
        rv+=buf;
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
    if(messageIdsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(messageIds);
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
  const std::vector<int64_t>& getMessageIds()const
  {
    if(!messageIdsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("messageIds");
    }
    return messageIds;
  }
  void setMessageIds(const std::vector<int64_t>& argValue)
  {
    messageIds=argValue;
    messageIdsFlag=true;
  }
  std::vector<int64_t>& getMessageIdsRef()
  {
    messageIdsFlag=true;
    return messageIds;
  }
  bool hasMessageIds()const
  {
    return messageIdsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryId");
    }
    if(!messageIdsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("messageIds");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(deliveryIdTag);
    ds.writeInt32LV(deliveryId); 
    ds.writeTag(messageIdsTag);
    ds.writeLength(DataStream::fieldSize(messageIds));
    for(std::vector<int64_t>::const_iterator it=messageIds.begin(),end=messageIds.end();it!=end;++it)
    {
      ds.writeInt64(*it);
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
    //  throw protogen::framework::IncompatibleVersionException("DropDeliveryMessages");
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
        case messageIdsTag:
        {
          if(messageIdsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("messageIds");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            messageIds.push_back(ds.readInt64());
            rd+=DataStream::fieldSize(messageIds.back());
          }
          messageIdsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DropDeliveryMessages",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryId");
    }
    if(!messageIdsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("messageIds");
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
  static const int32_t messageIdsTag=2;

  int32_t seqNum;
  int connId;

  int32_t deliveryId;
  std::vector<int64_t> messageIds;

  bool deliveryIdFlag;
  bool messageIdsFlag;
};

}
}
}
}
#endif
