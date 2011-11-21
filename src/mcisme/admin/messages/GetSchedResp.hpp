#ifndef __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_GETSCHEDRESP_HPP__
#define __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_GETSCHEDRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"


#ident "@(#) GetSchedResp version 1.0"



namespace smsc{
namespace mcisme{
namespace admin{
namespace messages{


class GetSchedResp:public Response{
public:
  GetSchedResp()
  {
    Clear();
  }
  void Clear()
  {
    Response::Clear();
    timeFlag=false;
    subscriberFlag=false;
    eventsCountFlag=false;
    lastErrorFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 9;
  }

  static std::string messageGetName()
  {
    return "GetSchedResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    rv+=Response::toString();
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
    if(subscriberFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="subscriber=";
      rv+=subscriber;
    }
    if(eventsCountFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="eventsCount=";
      sprintf(buf,"%d",(int)eventsCount);
      rv+=buf;
    }
    if(lastErrorFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="lastError=";
      sprintf(buf,"%d",lastError);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    rv+=Response::length<DataStream>();
    if(timeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(time);
    }
    if(subscriberFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(subscriber);
    }
    if(eventsCountFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(eventsCount);
    }
    if(lastErrorFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(lastError);
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
  const std::string& getSubscriber()const
  {
    if(!subscriberFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("subscriber");
    }
    return subscriber;
  }
  void setSubscriber(const std::string& argValue)
  {
    subscriber=argValue;
    subscriberFlag=true;
  }
  std::string& getSubscriberRef()
  {
    subscriberFlag=true;
    return subscriber;
  }
  bool hasSubscriber()const
  {
    return subscriberFlag;
  }
  int8_t getEventsCount()const
  {
    if(!eventsCountFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("eventsCount");
    }
    return eventsCount;
  }
  void setEventsCount(int8_t argValue)
  {
    eventsCount=argValue;
    eventsCountFlag=true;
  }
  int8_t& getEventsCountRef()
  {
    eventsCountFlag=true;
    return eventsCount;
  }
  bool hasEventsCount()const
  {
    return eventsCountFlag;
  }
  int32_t getLastError()const
  {
    if(!lastErrorFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("lastError");
    }
    return lastError;
  }
  void setLastError(int32_t argValue)
  {
    lastError=argValue;
    lastErrorFlag=true;
  }
  int32_t& getLastErrorRef()
  {
    lastErrorFlag=true;
    return lastError;
  }
  bool hasLastError()const
  {
    return lastErrorFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    Response::serialize(ds);
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    if(timeFlag)
    {
      ds.writeTag(timeTag);
    ds.writeInt64LV(time); 
    }
    if(subscriberFlag)
    {
      ds.writeTag(subscriberTag);
    ds.writeStrLV(subscriber); 
    }
    if(eventsCountFlag)
    {
      ds.writeTag(eventsCountTag);
    ds.writeByteLV(eventsCount); 
    }
    if(lastErrorFlag)
    {
      ds.writeTag(lastErrorTag);
    ds.writeInt32LV(lastError); 
    }
    ds.writeTag(DataStream::endOfMessage_tag);
  }

  template <class DataStream>
  void deserialize(DataStream& ds)
  {
    Clear();
    Response::deserialize(ds);
    bool endOfMessage=false;
    //int8_t rdVersionMajor=ds.readByte();
    //int8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("GetSchedResp");
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
        case subscriberTag:
        {
          if(subscriberFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("subscriber");
          }
          subscriber=ds.readStrLV();
          subscriberFlag=true;
        }break;
        case eventsCountTag:
        {
          if(eventsCountFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("eventsCount");
          }
          eventsCount=ds.readByteLV();
          eventsCountFlag=true;
        }break;
        case lastErrorTag:
        {
          if(lastErrorFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("lastError");
          }
          lastError=ds.readInt32LV();
          lastErrorFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetSchedResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }

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

  static const int32_t timeTag=1;
  static const int32_t subscriberTag=2;
  static const int32_t eventsCountTag=3;
  static const int32_t lastErrorTag=4;

  int connId;

  int64_t time;
  std::string subscriber;
  int8_t eventsCount;
  int32_t lastError;

  bool timeFlag;
  bool subscriberFlag;
  bool eventsCountFlag;
  bool lastErrorFlag;
};

}
}
}
}
#endif
