#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETNEXTMESSAGESPACK_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETNEXTMESSAGESPACK_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) GetNextMessagesPack version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class GetNextMessagesPack{
public:
  GetNextMessagesPack()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    reqIdFlag=false;
    countFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 45;
  }

  static std::string messageGetName()
  {
    return "GetNextMessagesPack";
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
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(reqIdTag);
    ds.writeInt32LV(reqId); 
    ds.writeTag(countTag);
    ds.writeInt32LV(count); 
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
    //  throw protogen::framework::IncompatibleVersionException("GetNextMessagesPack");
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
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetNextMessagesPack",tag);
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

  int32_t seqNum;
  int connId;

  int32_t reqId;
  int32_t count;

  bool reqIdFlag;
  bool countFlag;
};

}
}
}
}
#endif
