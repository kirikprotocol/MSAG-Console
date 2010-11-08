#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_MODIFYDELIVERYGLOSSARY_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_MODIFYDELIVERYGLOSSARY_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryGlossary.hpp"


#ident "@(#) ModifyDeliveryGlossary version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class ModifyDeliveryGlossary{
public:
  ModifyDeliveryGlossary()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    deliveryIdFlag=false;
    glossaryFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 40;
  }

  static std::string messageGetName()
  {
    return "ModifyDeliveryGlossary";
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
    if(glossaryFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="glossary=";
      rv+='(';
      rv+=glossary.toString();
      rv+=')';
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
    if(glossaryFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=glossary.length<DataStream>();
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
  const DeliveryGlossary& getGlossary()const
  {
    if(!glossaryFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("glossary");
    }
    return glossary;
  }
  void setGlossary(const DeliveryGlossary& argValue)
  {
    glossary=argValue;
    glossaryFlag=true;
  }
  DeliveryGlossary& getGlossaryRef()
  {
    glossaryFlag=true;
    return glossary;
  }
  bool hasGlossary()const
  {
    return glossaryFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryId");
    }
    if(!glossaryFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("glossary");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(deliveryIdTag);
    ds.writeInt32LV(deliveryId); 
    ds.writeTag(glossaryTag);
    ds.writeLength(glossary.length<DataStream>());
    glossary.serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("ModifyDeliveryGlossary");
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
        case glossaryTag:
        {
          if(glossaryFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("glossary");
          }

          ds.readLength();glossary.deserialize(ds);
          glossaryFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("ModifyDeliveryGlossary",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryId");
    }
    if(!glossaryFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("glossary");
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
  static const int32_t glossaryTag=2;

  int32_t seqNum;
  int connId;

  int32_t deliveryId;
  DeliveryGlossary glossary;

  bool deliveryIdFlag;
  bool glossaryFlag;
};

}
}
}
}
#endif
