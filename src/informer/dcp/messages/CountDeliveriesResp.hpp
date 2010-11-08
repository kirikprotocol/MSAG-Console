#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_COUNTDELIVERIESRESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_COUNTDELIVERIESRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) CountDeliveriesResp version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class CountDeliveriesResp{
public:
  CountDeliveriesResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    resultFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 55;
  }

  static std::string messageGetName()
  {
    return "CountDeliveriesResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(resultFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="result=";
      sprintf(buf,"%d",result);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(resultFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(result);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getResult()const
  {
    if(!resultFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("result");
    }
    return result;
  }
  void setResult(int32_t argValue)
  {
    result=argValue;
    resultFlag=true;
  }
  int32_t& getResultRef()
  {
    resultFlag=true;
    return result;
  }
  bool hasResult()const
  {
    return resultFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!resultFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("result");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(resultTag);
    ds.writeInt32LV(result); 
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
    //  throw protogen::framework::IncompatibleVersionException("CountDeliveriesResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case resultTag:
        {
          if(resultFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("result");
          }
          result=ds.readInt32LV();
          resultFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("CountDeliveriesResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!resultFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("result");
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

  static const int32_t resultTag=1;

  int32_t seqNum;
  int connId;

  int32_t result;

  bool resultFlag;
};

}
}
}
}
#endif
