#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_COUNTDELIVERIESNEXTRESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_COUNTDELIVERIESNEXTRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) CountDeliveriesNextResp version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class CountDeliveriesNextResp{
public:
  CountDeliveriesNextResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    moreDeliveriesFlag=false;
    countFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 61;
  }

  static std::string messageGetName()
  {
    return "CountDeliveriesNextResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(moreDeliveriesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="moreDeliveries=";
      rv+=moreDeliveries?"true":"false";
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
    if(moreDeliveriesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(moreDeliveries);
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
  bool getMoreDeliveries()const
  {
    if(!moreDeliveriesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("moreDeliveries");
    }
    return moreDeliveries;
  }
  void setMoreDeliveries(bool argValue)
  {
    moreDeliveries=argValue;
    moreDeliveriesFlag=true;
  }
  bool& getMoreDeliveriesRef()
  {
    moreDeliveriesFlag=true;
    return moreDeliveries;
  }
  bool hasMoreDeliveries()const
  {
    return moreDeliveriesFlag;
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
    if(!moreDeliveriesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("moreDeliveries");
    }
    if(!countFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("count");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(moreDeliveriesTag);
    ds.writeBoolLV(moreDeliveries); 
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
    //  throw protogen::framework::IncompatibleVersionException("CountDeliveriesNextResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case moreDeliveriesTag:
        {
          if(moreDeliveriesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("moreDeliveries");
          }
          moreDeliveries=ds.readBoolLV();
          moreDeliveriesFlag=true;
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
          //  throw protogen::framework::UnexpectedTag("CountDeliveriesNextResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!moreDeliveriesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("moreDeliveries");
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

  static const int32_t moreDeliveriesTag=1;
  static const int32_t countTag=2;

  int32_t seqNum;
  int connId;

  bool moreDeliveries;
  int32_t count;

  bool moreDeliveriesFlag;
  bool countFlag;
};

}
}
}
}
#endif
