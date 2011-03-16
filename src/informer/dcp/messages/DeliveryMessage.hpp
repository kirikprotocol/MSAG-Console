#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYMESSAGE_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYMESSAGE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "MessageType.hpp"
#include "MessageFlags.hpp"


#ident "@(#) DeliveryMessage version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class DeliveryMessage{
public:
  DeliveryMessage()
  {
    Clear();
  }
  void Clear()
  {
    abonentFlag=false;
    msgTypeFlag=false;
    textFlag=false;
    indexFlag=false;
    userDataFlag=false;
    flagsFlag=false;
  }
 

  static std::string messageGetName()
  {
    return "DeliveryMessage";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(abonentFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="abonent=";
      rv+=abonent;
    }
    if(msgTypeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="msgType=";
      rv+=MessageType::getNameByValue(msgType);
    }
    if(textFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="text=";
      rv+=text;
    }
    if(indexFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="index=";
      sprintf(buf,"%d",index);
      rv+=buf;
    }
    if(userDataFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="userData=";
      rv+=userData;
    }
    if(flagsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flags=";
      rv+='(';
      rv+=flags.toString();
      rv+=')';
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(abonentFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(abonent);
    }
    if(msgTypeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(msgType.getValue());
 
    }
    if(textFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(text);
    }
    if(indexFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(index);
    }
    if(userDataFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(userData);
    }
    if(flagsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=flags.length<DataStream>();
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getAbonent()const
  {
    if(!abonentFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("abonent");
    }
    return abonent;
  }
  void setAbonent(const std::string& argValue)
  {
    abonent=argValue;
    abonentFlag=true;
  }
  std::string& getAbonentRef()
  {
    abonentFlag=true;
    return abonent;
  }
  bool hasAbonent()const
  {
    return abonentFlag;
  }
  const MessageType& getMsgType()const
  {
    if(!msgTypeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("msgType");
    }
    return msgType;
  }
  void setMsgType(const MessageType& argValue)
  {
    msgType=argValue;
    msgTypeFlag=true;
  }
  MessageType& getMsgTypeRef()
  {
    msgTypeFlag=true;
    return msgType;
  }
  bool hasMsgType()const
  {
    return msgTypeFlag;
  }
  const std::string& getText()const
  {
    if(!textFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("text");
    }
    return text;
  }
  void setText(const std::string& argValue)
  {
    text=argValue;
    textFlag=true;
  }
  std::string& getTextRef()
  {
    textFlag=true;
    return text;
  }
  bool hasText()const
  {
    return textFlag;
  }
  int32_t getIndex()const
  {
    if(!indexFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("index");
    }
    return index;
  }
  void setIndex(int32_t argValue)
  {
    index=argValue;
    indexFlag=true;
  }
  int32_t& getIndexRef()
  {
    indexFlag=true;
    return index;
  }
  bool hasIndex()const
  {
    return indexFlag;
  }
  const std::string& getUserData()const
  {
    if(!userDataFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("userData");
    }
    return userData;
  }
  void setUserData(const std::string& argValue)
  {
    userData=argValue;
    userDataFlag=true;
  }
  std::string& getUserDataRef()
  {
    userDataFlag=true;
    return userData;
  }
  bool hasUserData()const
  {
    return userDataFlag;
  }
  const MessageFlags& getFlags()const
  {
    if(!flagsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("flags");
    }
    return flags;
  }
  void setFlags(const MessageFlags& argValue)
  {
    flags=argValue;
    flagsFlag=true;
  }
  MessageFlags& getFlagsRef()
  {
    flagsFlag=true;
    return flags;
  }
  bool hasFlags()const
  {
    return flagsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!abonentFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("abonent");
    }
    if(!msgTypeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("msgType");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(abonentTag);
    ds.writeStrLV(abonent); 
    ds.writeTag(msgTypeTag);
    ds.writeByteLV(msgType.getValue());
 
    if(textFlag)
    {
      ds.writeTag(textTag);
    ds.writeStrLV(text); 
    }
    if(indexFlag)
    {
      ds.writeTag(indexTag);
    ds.writeInt32LV(index); 
    }
    if(userDataFlag)
    {
      ds.writeTag(userDataTag);
    ds.writeStrLV(userData); 
    }
    if(flagsFlag)
    {
      ds.writeTag(flagsTag);
    ds.writeLength(flags.length<DataStream>());
    flags.serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("DeliveryMessage");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case abonentTag:
        {
          if(abonentFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("abonent");
          }
          abonent=ds.readStrLV();
          abonentFlag=true;
        }break;
        case msgTypeTag:
        {
          if(msgTypeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("msgType");
          }
          msgType=ds.readByteLV();
          msgTypeFlag=true;
        }break;
        case textTag:
        {
          if(textFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("text");
          }
          text=ds.readStrLV();
          textFlag=true;
        }break;
        case indexTag:
        {
          if(indexFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("index");
          }
          index=ds.readInt32LV();
          indexFlag=true;
        }break;
        case userDataTag:
        {
          if(userDataFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("userData");
          }
          userData=ds.readStrLV();
          userDataFlag=true;
        }break;
        case flagsTag:
        {
          if(flagsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("flags");
          }

          ds.readLength();flags.deserialize(ds);
          flagsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DeliveryMessage",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!abonentFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("abonent");
    }
    if(!msgTypeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("msgType");
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

  static const int32_t abonentTag=1;
  static const int32_t msgTypeTag=2;
  static const int32_t textTag=3;
  static const int32_t indexTag=4;
  static const int32_t userDataTag=5;
  static const int32_t flagsTag=6;

  int connId;

  std::string abonent;
  MessageType msgType;
  std::string text;
  int32_t index;
  std::string userData;
  MessageFlags flags;

  bool abonentFlag;
  bool msgTypeFlag;
  bool textFlag;
  bool indexFlag;
  bool userDataFlag;
  bool flagsFlag;
};

}
}
}
}
#endif
