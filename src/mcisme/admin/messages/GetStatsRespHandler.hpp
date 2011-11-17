#ifndef __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_GETSTATSRESP_HPP__
#define __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_GETSTATSRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"


#ident "@(#) GetStatsResp version 1.0"



namespace smsc{
namespace mcisme{
namespace admin{
namespace messages{


class GetStatsResp:public Response{
public:
  GetStatsResp()
  {
    Clear();
  }
  void Clear()
  {
    Response::Clear();
    missedFlag=false;
    deliveredFlag=false;
    failedFlag=false;
    notifiedFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 5;
  }

  static std::string messageGetName()
  {
    return "GetStatsResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    rv+=Response::toString();
    if(missedFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="missed=";
      sprintf(buf,"%d",missed);
      rv+=buf;
    }
    if(deliveredFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="delivered=";
      sprintf(buf,"%d",delivered);
      rv+=buf;
    }
    if(failedFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="failed=";
      sprintf(buf,"%d",failed);
      rv+=buf;
    }
    if(notifiedFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="notified=";
      sprintf(buf,"%d",notified);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    rv+=Response::length<DataStream>();
    if(missedFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(missed);
    }
    if(deliveredFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(delivered);
    }
    if(failedFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(failed);
    }
    if(notifiedFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(notified);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getMissed()const
  {
    if(!missedFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("missed");
    }
    return missed;
  }
  void setMissed(int32_t argValue)
  {
    missed=argValue;
    missedFlag=true;
  }
  int32_t& getMissedRef()
  {
    missedFlag=true;
    return missed;
  }
  bool hasMissed()const
  {
    return missedFlag;
  }
  int32_t getDelivered()const
  {
    if(!deliveredFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("delivered");
    }
    return delivered;
  }
  void setDelivered(int32_t argValue)
  {
    delivered=argValue;
    deliveredFlag=true;
  }
  int32_t& getDeliveredRef()
  {
    deliveredFlag=true;
    return delivered;
  }
  bool hasDelivered()const
  {
    return deliveredFlag;
  }
  int32_t getFailed()const
  {
    if(!failedFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("failed");
    }
    return failed;
  }
  void setFailed(int32_t argValue)
  {
    failed=argValue;
    failedFlag=true;
  }
  int32_t& getFailedRef()
  {
    failedFlag=true;
    return failed;
  }
  bool hasFailed()const
  {
    return failedFlag;
  }
  int32_t getNotified()const
  {
    if(!notifiedFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("notified");
    }
    return notified;
  }
  void setNotified(int32_t argValue)
  {
    notified=argValue;
    notifiedFlag=true;
  }
  int32_t& getNotifiedRef()
  {
    notifiedFlag=true;
    return notified;
  }
  bool hasNotified()const
  {
    return notifiedFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    Response::serialize(ds);
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    if(missedFlag)
    {
      ds.writeTag(missedTag);
    ds.writeInt32LV(missed); 
    }
    if(deliveredFlag)
    {
      ds.writeTag(deliveredTag);
    ds.writeInt32LV(delivered); 
    }
    if(failedFlag)
    {
      ds.writeTag(failedTag);
    ds.writeInt32LV(failed); 
    }
    if(notifiedFlag)
    {
      ds.writeTag(notifiedTag);
    ds.writeInt32LV(notified); 
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
    //  throw protogen::framework::IncompatibleVersionException("GetStatsResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case missedTag:
        {
          if(missedFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("missed");
          }
          missed=ds.readInt32LV();
          missedFlag=true;
        }break;
        case deliveredTag:
        {
          if(deliveredFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("delivered");
          }
          delivered=ds.readInt32LV();
          deliveredFlag=true;
        }break;
        case failedTag:
        {
          if(failedFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("failed");
          }
          failed=ds.readInt32LV();
          failedFlag=true;
        }break;
        case notifiedTag:
        {
          if(notifiedFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("notified");
          }
          notified=ds.readInt32LV();
          notifiedFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetStatsResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t missedTag=1;
  static const int32_t deliveredTag=2;
  static const int32_t failedTag=3;
  static const int32_t notifiedTag=4;


  int32_t missed;
  int32_t delivered;
  int32_t failed;
  int32_t notified;

  bool missedFlag;
  bool deliveredFlag;
  bool failedFlag;
  bool notifiedFlag;
};

}
}
}
}
#endif
