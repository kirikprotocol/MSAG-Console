#ifndef __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_TIMELINEINFO_HPP__
#define __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_TIMELINEINFO_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) TimeLineInfo version 1.0"



namespace smsc{
namespace mcisme{
namespace admin{
namespace messages{


class TimeLineInfo{
public:
  TimeLineInfo()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    timeFlag=false;
    countFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 10;
  }

  static std::string messageGetName()
  {
    return "TimeLineInfo";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(timeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="time=";
      sprintf(buf,"%lld",time);
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
    if(timeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(time);
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
  int64_t getTime()const
  {
    if(!timeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("time");
    }
    return time;
  }
  void setTime(int64_t argValue)
  {
    time=argValue;
    timeFlag=true;
  }
  int64_t& getTimeRef()
  {
    timeFlag=true;
    return time;
  }
  bool hasTime()const
  {
    return timeFlag;
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
    if(!timeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("time");
    }
    if(!countFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("count");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(timeTag);
    ds.writeInt64LV(time); 
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
    //  throw protogen::framework::IncompatibleVersionException("TimeLineInfo");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case timeTag:
        {
          if(timeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("time");
          }
          time=ds.readInt64LV();
          timeFlag=true;
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
          //  throw protogen::framework::UnexpectedTag("TimeLineInfo",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!timeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("time");
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

 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t timeTag=1;
  static const int32_t countTag=2;

  int32_t seqNum;

  int64_t time;
  int32_t count;

  bool timeFlag;
  bool countFlag;
};

}
}
}
}
#endif
