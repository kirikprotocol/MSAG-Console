#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMEREMOVE_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMEREMOVE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) SmeRemove version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class SmeRemove{
public:
  SmeRemove()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    smeIdFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 20;
  }

  static std::string messageGetName()
  {
    return "SmeRemove";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(smeIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="smeId=";
      rv+=smeId;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(smeIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(smeId);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getSmeId()const
  {
    if(!smeIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("smeId");
    }
    return smeId;
  }
  void setSmeId(const std::string& argValue)
  {
    smeId=argValue;
    smeIdFlag=true;
  }
  std::string& getSmeIdRef()
  {
    smeIdFlag=true;
    return smeId;
  }
  bool hasSmeId()const
  {
    return smeIdFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!smeIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("smeId");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(smeIdTag);
    ds.writeStrLV(smeId); 
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
    //  throw protogen::framework::IncompatibleVersionException("SmeRemove");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case smeIdTag:
        {
          if(smeIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("smeId");
          }
          smeId=ds.readStrLV();
          smeIdFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("SmeRemove",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!smeIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("smeId");
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

 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t smeIdTag=1;

  int32_t seqNum;

  std::string smeId;

  bool smeIdFlag;
};

}
}
}
}
#endif
