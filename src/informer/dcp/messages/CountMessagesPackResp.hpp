#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_COUNTMESSAGESPACKRESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_COUNTMESSAGESPACKRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) CountMessagesPackResp version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class CountMessagesPackResp{
public:
  CountMessagesPackResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    moreMessagesFlag=false;
    countFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 63;
  }

  static std::string messageGetName()
  {
    return "CountMessagesPackResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(moreMessagesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="moreMessages=";
      rv+=moreMessages?"true":"false";
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
    if(moreMessagesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(moreMessages);
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
  bool getMoreMessages()const
  {
    if(!moreMessagesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("moreMessages");
    }
    return moreMessages;
  }
  void setMoreMessages(bool argValue)
  {
    moreMessages=argValue;
    moreMessagesFlag=true;
  }
  bool& getMoreMessagesRef()
  {
    moreMessagesFlag=true;
    return moreMessages;
  }
  bool hasMoreMessages()const
  {
    return moreMessagesFlag;
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
    if(!moreMessagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("moreMessages");
    }
    if(!countFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("count");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(moreMessagesTag);
    ds.writeBoolLV(moreMessages); 
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
    //  throw protogen::framework::IncompatibleVersionException("CountMessagesPackResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case moreMessagesTag:
        {
          if(moreMessagesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("moreMessages");
          }
          moreMessages=ds.readBoolLV();
          moreMessagesFlag=true;
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
          //  throw protogen::framework::UnexpectedTag("CountMessagesPackResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!moreMessagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("moreMessages");
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

  static const int32_t moreMessagesTag=1;
  static const int32_t countTag=2;

  int32_t seqNum;
  int connId;

  bool moreMessages;
  int32_t count;

  bool moreMessagesFlag;
  bool countFlag;
};

}
}
}
}
#endif
