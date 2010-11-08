#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETDELIVERYGLOSSARYRESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETDELIVERYGLOSSARYRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryGlossary.hpp"


#ident "@(#) GetDeliveryGlossaryResp version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class GetDeliveryGlossaryResp{
public:
  GetDeliveryGlossaryResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    glossaryFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 39;
  }

  static std::string messageGetName()
  {
    return "GetDeliveryGlossaryResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
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
    if(glossaryFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=glossary.length<DataStream>();
    }
    rv+=DataStream::tagTypeSize;
    return rv;
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
    if(!glossaryFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("glossary");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
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
    //  throw protogen::framework::IncompatibleVersionException("GetDeliveryGlossaryResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
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
          //  throw protogen::framework::UnexpectedTag("GetDeliveryGlossaryResp",tag);
          //}
          ds.skip(ds.readLength());
      }
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

  static const int32_t glossaryTag=1;

  int32_t seqNum;
  int connId;

  DeliveryGlossary glossary;

  bool glossaryFlag;
};

}
}
}
}
#endif
