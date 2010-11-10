#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_SENDTESTSMS_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_SENDTESTSMS_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryMode.hpp"


#ident "@(#) SendTestSms version 1.0"



namespace eyeline{
namespace informer{
namespace admin{
namespace messages{


class SendTestSms{
public:
  SendTestSms()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    sourceAddrFlag=false;
    abonentFlag=false;
    textFlag=false;
    flashFlag=false;
    deliveryModeFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 9;
  }

  static std::string messageGetName()
  {
    return "SendTestSms";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(sourceAddrFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="sourceAddr=";
      rv+=sourceAddr;
    }
    if(abonentFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="abonent=";
      rv+=abonent;
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
    if(flashFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flash=";
      rv+=flash?"true":"false";
    }
    if(deliveryModeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="deliveryMode=";
      rv+=DeliveryMode::getNameByValue(deliveryMode);
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(sourceAddrFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(sourceAddr);
    }
    if(abonentFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(abonent);
    }
    if(textFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(text);
    }
    if(flashFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(flash);
    }
    if(deliveryModeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(deliveryMode.getValue());
 
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getSourceAddr()const
  {
    if(!sourceAddrFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("sourceAddr");
    }
    return sourceAddr;
  }
  void setSourceAddr(const std::string& argValue)
  {
    sourceAddr=argValue;
    sourceAddrFlag=true;
  }
  std::string& getSourceAddrRef()
  {
    sourceAddrFlag=true;
    return sourceAddr;
  }
  bool hasSourceAddr()const
  {
    return sourceAddrFlag;
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
  bool getFlash()const
  {
    if(!flashFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("flash");
    }
    return flash;
  }
  void setFlash(bool argValue)
  {
    flash=argValue;
    flashFlag=true;
  }
  bool& getFlashRef()
  {
    flashFlag=true;
    return flash;
  }
  bool hasFlash()const
  {
    return flashFlag;
  }
  const DeliveryMode& getDeliveryMode()const
  {
    if(!deliveryModeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("deliveryMode");
    }
    return deliveryMode;
  }
  void setDeliveryMode(const DeliveryMode& argValue)
  {
    deliveryMode=argValue;
    deliveryModeFlag=true;
  }
  DeliveryMode& getDeliveryModeRef()
  {
    deliveryModeFlag=true;
    return deliveryMode;
  }
  bool hasDeliveryMode()const
  {
    return deliveryModeFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!sourceAddrFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("sourceAddr");
    }
    if(!abonentFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("abonent");
    }
    if(!textFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("text");
    }
    if(!flashFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flash");
    }
    if(!deliveryModeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryMode");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(sourceAddrTag);
    ds.writeStrLV(sourceAddr); 
    ds.writeTag(abonentTag);
    ds.writeStrLV(abonent); 
    ds.writeTag(textTag);
    ds.writeStrLV(text); 
    ds.writeTag(flashTag);
    ds.writeBoolLV(flash); 
    ds.writeTag(deliveryModeTag);
    ds.writeByteLV(deliveryMode.getValue());
 
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
    //  throw protogen::framework::IncompatibleVersionException("SendTestSms");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case sourceAddrTag:
        {
          if(sourceAddrFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("sourceAddr");
          }
          sourceAddr=ds.readStrLV();
          sourceAddrFlag=true;
        }break;
        case abonentTag:
        {
          if(abonentFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("abonent");
          }
          abonent=ds.readStrLV();
          abonentFlag=true;
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
        case flashTag:
        {
          if(flashFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("flash");
          }
          flash=ds.readBoolLV();
          flashFlag=true;
        }break;
        case deliveryModeTag:
        {
          if(deliveryModeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("deliveryMode");
          }
          deliveryMode=ds.readByteLV();
          deliveryModeFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("SendTestSms",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!sourceAddrFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("sourceAddr");
    }
    if(!abonentFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("abonent");
    }
    if(!textFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("text");
    }
    if(!flashFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flash");
    }
    if(!deliveryModeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryMode");
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

  static const int32_t sourceAddrTag=1;
  static const int32_t abonentTag=2;
  static const int32_t textTag=3;
  static const int32_t flashTag=4;
  static const int32_t deliveryModeTag=5;

  int32_t seqNum;
  int connId;

  std::string sourceAddr;
  std::string abonent;
  std::string text;
  bool flash;
  DeliveryMode deliveryMode;

  bool sourceAddrFlag;
  bool abonentFlag;
  bool textFlag;
  bool flashFlag;
  bool deliveryModeFlag;
};

}
}
}
}
#endif
