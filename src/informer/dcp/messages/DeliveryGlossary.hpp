#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYGLOSSARY_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYGLOSSARY_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) DeliveryGlossary version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class DeliveryGlossary{
public:
  DeliveryGlossary()
  {
    Clear();
  }
  void Clear()
  {
    messagesFlag=false;
    messages.clear();
  }
 

  static std::string messageGetName()
  {
    return "DeliveryGlossary";
  }

  std::string toString()const
  {
    std::string rv;
    if(messagesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="messages=";
      rv+="[";
      bool first=true;
      for(std::vector<std::string>::const_iterator it=messages.begin(),end=messages.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=*it;
      }
      rv+="]";
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(messagesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(messages);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::vector<std::string>& getMessages()const
  {
    if(!messagesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("messages");
    }
    return messages;
  }
  void setMessages(const std::vector<std::string>& argValue)
  {
    messages=argValue;
    messagesFlag=true;
  }
  std::vector<std::string>& getMessagesRef()
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
    if(!messagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("messages");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(messagesTag);
    ds.writeLength(DataStream::fieldSize(messages));
    for(std::vector<std::string>::const_iterator it=messages.begin(),end=messages.end();it!=end;++it)
    {
      ds.writeStr(*it);
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
    //  throw protogen::framework::IncompatibleVersionException("DeliveryGlossary");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case messagesTag:
        {
          if(messagesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("messages");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            messages.push_back(ds.readStr());
            rd+=DataStream::fieldSize(messages.back());
            rd+=DataStream::lengthTypeSize;
          }
          messagesFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DeliveryGlossary",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!messagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("messages");
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

  static const int32_t messagesTag=1;

  int connId;

  std::vector<std::string> messages;

  bool messagesFlag;
};

}
}
}
}
#endif
