#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_COUNTDELIVERIESNEXT_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_COUNTDELIVERIESNEXT_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) CountDeliveriesNext version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class CountDeliveriesNext{
public:
  CountDeliveriesNext()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    reqIdFlag=false;
    countFlag=false;
    timeoutFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 60;
  }

  static std::string messageGetName()
  {
    return "CountDeliveriesNext";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(reqIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="reqId=";
      sprintf(buf,"%d",reqId);
      rv+=buf;
    }
    if(countFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="count=";
      sprintf(buf,"%d",count);
      rv+=buf;
    }
    if(timeoutFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="timeout=";
      sprintf(buf,"%d",timeout);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(reqIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(reqId);
    }
    if(countFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(count);
    }
    if(timeoutFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(timeout);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getReqId()const
  {
    if(!reqIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("reqId");
    }
    return reqId;
  }
  void setReqId(int32_t argValue)
  {
    reqId=argValue;
    reqIdFlag=true;
  }
  int32_t& getReqIdRef()
  {
    reqIdFlag=true;
    return reqId;
  }
  bool hasReqId()const
  {
    return reqIdFlag;
  }
  int32_t getCount()const
  {
    if(!countFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("count");
    }
    return count;
  }
  void setCount(int32_t argValue)
  {
    count=argValue;
    countFlag=true;
  }
  int32_t& getCountRef()
  {
    countFlag=true;
    return count;
  }
  bool hasCount()const
  {
    return countFlag;
  }
  int32_t getTimeout()const
  {
    if(!timeoutFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("timeout");
    }
    return timeout;
  }
  void setTimeout(int32_t argValue)
  {
    timeout=argValue;
    timeoutFlag=true;
  }
  int32_t& getTimeoutRef()
  {
    timeoutFlag=true;
    return timeout;
  }
  bool hasTimeout()const
  {
    return timeoutFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!reqIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("reqId");
    }
    if(!countFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("count");
    }
    if(!timeoutFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("timeout");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(reqIdTag);
    ds.writeInt32LV(reqId); 
    ds.writeTag(countTag);
    ds.writeInt32LV(count); 
    ds.writeTag(timeoutTag);
    ds.writeInt32LV(timeout); 
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
    //  throw protogen::framework::IncompatibleVersionException("CountDeliveriesNext");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case reqIdTag:
        {
          if(reqIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("reqId");
          }
          reqId=ds.readInt32LV();
          reqIdFlag=true;
        }break;
        case countTag:
        {
          if(countFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("count");
          }
          count=ds.readInt32LV();
          countFlag=true;
        }break;
        case timeoutTag:
        {
          if(timeoutFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("timeout");
          }
          timeout=ds.readInt32LV();
          timeoutFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("CountDeliveriesNext",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!reqIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("reqId");
    }
    if(!countFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("count");
    }
    if(!timeoutFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("timeout");
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

  static const int32_t reqIdTag=1;
  static const int32_t countTag=2;
  static const int32_t timeoutTag=3;

  int32_t seqNum;
  int connId;

  int32_t reqId;
  int32_t count;
  int32_t timeout;

  bool reqIdFlag;
  bool countFlag;
  bool timeoutFlag;
};

}
}
}
}
#endif
