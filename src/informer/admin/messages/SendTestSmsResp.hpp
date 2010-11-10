#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_SENDTESTSMSRESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_SENDTESTSMSRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) SendTestSmsResp version 1.0"



namespace eyeline{
namespace informer{
namespace admin{
namespace messages{


class SendTestSmsResp{
public:
  SendTestSmsResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    respCodeFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 10;
  }

  static std::string messageGetName()
  {
    return "SendTestSmsResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(respCodeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="respCode=";
      sprintf(buf,"%d",respCode);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(respCodeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(respCode);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getRespCode()const
  {
    if(!respCodeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("respCode");
    }
    return respCode;
  }
  void setRespCode(int32_t argValue)
  {
    respCode=argValue;
    respCodeFlag=true;
  }
  int32_t& getRespCodeRef()
  {
    respCodeFlag=true;
    return respCode;
  }
  bool hasRespCode()const
  {
    return respCodeFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!respCodeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("respCode");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(respCodeTag);
    ds.writeInt32LV(respCode); 
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
    //  throw protogen::framework::IncompatibleVersionException("SendTestSmsResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case respCodeTag:
        {
          if(respCodeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("respCode");
          }
          respCode=ds.readInt32LV();
          respCodeFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("SendTestSmsResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!respCodeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("respCode");
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

  static const int32_t respCodeTag=1;

  int32_t seqNum;
  int connId;

  int32_t respCode;

  bool respCodeFlag;
};

}
}
}
}
#endif
