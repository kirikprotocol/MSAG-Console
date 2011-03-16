#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_MESSAGEFLAGS_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_MESSAGEFLAGS_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryMode.hpp"


#ident "@(#) MessageFlags version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class MessageFlags{
public:
  MessageFlags()
  {
    Clear();
  }
  void Clear()
  {
    transactionModeFlag=false;
    useDataSmFlag=false;
    replaceMessageFlag=false;
    flashFlag=false;
    svcTypeFlag=false;
    sourceAddressFlag=false;
    deliveryModeFlag=false;
  }
 

  static std::string messageGetName()
  {
    return "MessageFlags";
  }

  std::string toString()const
  {
    std::string rv;
    if(transactionModeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="transactionMode=";
      rv+=transactionMode?"true":"false";
    }
    if(useDataSmFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="useDataSm=";
      rv+=useDataSm?"true":"false";
    }
    if(replaceMessageFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="replaceMessage=";
      rv+=replaceMessage?"true":"false";
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
    if(svcTypeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="svcType=";
      rv+=svcType;
    }
    if(sourceAddressFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="sourceAddress=";
      rv+=sourceAddress;
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
    if(transactionModeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(transactionMode);
    }
    if(useDataSmFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(useDataSm);
    }
    if(replaceMessageFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(replaceMessage);
    }
    if(flashFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(flash);
    }
    if(svcTypeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(svcType);
    }
    if(sourceAddressFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(sourceAddress);
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
  bool getTransactionMode()const
  {
    if(!transactionModeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("transactionMode");
    }
    return transactionMode;
  }
  void setTransactionMode(bool argValue)
  {
    transactionMode=argValue;
    transactionModeFlag=true;
  }
  bool& getTransactionModeRef()
  {
    transactionModeFlag=true;
    return transactionMode;
  }
  bool hasTransactionMode()const
  {
    return transactionModeFlag;
  }
  bool getUseDataSm()const
  {
    if(!useDataSmFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("useDataSm");
    }
    return useDataSm;
  }
  void setUseDataSm(bool argValue)
  {
    useDataSm=argValue;
    useDataSmFlag=true;
  }
  bool& getUseDataSmRef()
  {
    useDataSmFlag=true;
    return useDataSm;
  }
  bool hasUseDataSm()const
  {
    return useDataSmFlag;
  }
  bool getReplaceMessage()const
  {
    if(!replaceMessageFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("replaceMessage");
    }
    return replaceMessage;
  }
  void setReplaceMessage(bool argValue)
  {
    replaceMessage=argValue;
    replaceMessageFlag=true;
  }
  bool& getReplaceMessageRef()
  {
    replaceMessageFlag=true;
    return replaceMessage;
  }
  bool hasReplaceMessage()const
  {
    return replaceMessageFlag;
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
  const std::string& getSvcType()const
  {
    if(!svcTypeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("svcType");
    }
    return svcType;
  }
  void setSvcType(const std::string& argValue)
  {
    svcType=argValue;
    svcTypeFlag=true;
  }
  std::string& getSvcTypeRef()
  {
    svcTypeFlag=true;
    return svcType;
  }
  bool hasSvcType()const
  {
    return svcTypeFlag;
  }
  const std::string& getSourceAddress()const
  {
    if(!sourceAddressFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("sourceAddress");
    }
    return sourceAddress;
  }
  void setSourceAddress(const std::string& argValue)
  {
    sourceAddress=argValue;
    sourceAddressFlag=true;
  }
  std::string& getSourceAddressRef()
  {
    sourceAddressFlag=true;
    return sourceAddress;
  }
  bool hasSourceAddress()const
  {
    return sourceAddressFlag;
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
    if(!transactionModeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("transactionMode");
    }
    if(!useDataSmFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("useDataSm");
    }
    if(!replaceMessageFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("replaceMessage");
    }
    if(!flashFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flash");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(transactionModeTag);
    ds.writeBoolLV(transactionMode); 
    ds.writeTag(useDataSmTag);
    ds.writeBoolLV(useDataSm); 
    ds.writeTag(replaceMessageTag);
    ds.writeBoolLV(replaceMessage); 
    ds.writeTag(flashTag);
    ds.writeBoolLV(flash); 
    if(svcTypeFlag)
    {
      ds.writeTag(svcTypeTag);
    ds.writeStrLV(svcType); 
    }
    if(sourceAddressFlag)
    {
      ds.writeTag(sourceAddressTag);
    ds.writeStrLV(sourceAddress); 
    }
    if(deliveryModeFlag)
    {
      ds.writeTag(deliveryModeTag);
    ds.writeByteLV(deliveryMode.getValue());
 
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
    //  throw protogen::framework::IncompatibleVersionException("MessageFlags");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case transactionModeTag:
        {
          if(transactionModeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("transactionMode");
          }
          transactionMode=ds.readBoolLV();
          transactionModeFlag=true;
        }break;
        case useDataSmTag:
        {
          if(useDataSmFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("useDataSm");
          }
          useDataSm=ds.readBoolLV();
          useDataSmFlag=true;
        }break;
        case replaceMessageTag:
        {
          if(replaceMessageFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("replaceMessage");
          }
          replaceMessage=ds.readBoolLV();
          replaceMessageFlag=true;
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
        case svcTypeTag:
        {
          if(svcTypeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("svcType");
          }
          svcType=ds.readStrLV();
          svcTypeFlag=true;
        }break;
        case sourceAddressTag:
        {
          if(sourceAddressFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("sourceAddress");
          }
          sourceAddress=ds.readStrLV();
          sourceAddressFlag=true;
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
          //  throw protogen::framework::UnexpectedTag("MessageFlags",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!transactionModeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("transactionMode");
    }
    if(!useDataSmFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("useDataSm");
    }
    if(!replaceMessageFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("replaceMessage");
    }
    if(!flashFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flash");
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

  static const int32_t transactionModeTag=1;
  static const int32_t useDataSmTag=2;
  static const int32_t replaceMessageTag=3;
  static const int32_t flashTag=4;
  static const int32_t svcTypeTag=5;
  static const int32_t sourceAddressTag=6;
  static const int32_t deliveryModeTag=7;

  int connId;

  bool transactionMode;
  bool useDataSm;
  bool replaceMessage;
  bool flash;
  std::string svcType;
  std::string sourceAddress;
  DeliveryMode deliveryMode;

  bool transactionModeFlag;
  bool useDataSmFlag;
  bool replaceMessageFlag;
  bool flashFlag;
  bool svcTypeFlag;
  bool sourceAddressFlag;
  bool deliveryModeFlag;
};

}
}
}
}
#endif
