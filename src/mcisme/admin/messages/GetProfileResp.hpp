#ifndef __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_GETPROFILERESP_HPP__
#define __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_GETPROFILERESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"


#ident "@(#) GetProfileResp version 1.0"



namespace smsc{
namespace mcisme{
namespace admin{
namespace messages{


class GetProfileResp:public Response{
public:
  GetProfileResp()
  {
    Clear();
  }
  void Clear()
  {
    Response::Clear();
    eventMaskFlag=false;
    informFlag=false;
    notifyFlag=false;
    wantNotifyMeFlag=false;
    informTemplateIdFlag=false;
    notifyTemplateIdFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 15;
  }

  static std::string messageGetName()
  {
    return "GetProfileResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    rv+=Response::toString();
    if(eventMaskFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="eventMask=";
      sprintf(buf,"%d",(int)eventMask);
      rv+=buf;
    }
    if(informFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="inform=";
      rv+=inform?"true":"false";
    }
    if(notifyFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="notify=";
      rv+=notify?"true":"false";
    }
    if(wantNotifyMeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="wantNotifyMe=";
      rv+=wantNotifyMe?"true":"false";
    }
    if(informTemplateIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="informTemplateId=";
      sprintf(buf,"%d",(int)informTemplateId);
      rv+=buf;
    }
    if(notifyTemplateIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="notifyTemplateId=";
      sprintf(buf,"%d",(int)notifyTemplateId);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    rv+=Response::length<DataStream>();
    if(eventMaskFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(eventMask);
    }
    if(informFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(inform);
    }
    if(notifyFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(notify);
    }
    if(wantNotifyMeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(wantNotifyMe);
    }
    if(informTemplateIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(informTemplateId);
    }
    if(notifyTemplateIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(notifyTemplateId);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int8_t getEventMask()const
  {
    if(!eventMaskFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("eventMask");
    }
    return eventMask;
  }
  void setEventMask(int8_t argValue)
  {
    eventMask=argValue;
    eventMaskFlag=true;
  }
  int8_t& getEventMaskRef()
  {
    eventMaskFlag=true;
    return eventMask;
  }
  bool hasEventMask()const
  {
    return eventMaskFlag;
  }
  bool getInform()const
  {
    if(!informFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("inform");
    }
    return inform;
  }
  void setInform(bool argValue)
  {
    inform=argValue;
    informFlag=true;
  }
  bool& getInformRef()
  {
    informFlag=true;
    return inform;
  }
  bool hasInform()const
  {
    return informFlag;
  }
  bool getNotify()const
  {
    if(!notifyFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("notify");
    }
    return notify;
  }
  void setNotify(bool argValue)
  {
    notify=argValue;
    notifyFlag=true;
  }
  bool& getNotifyRef()
  {
    notifyFlag=true;
    return notify;
  }
  bool hasNotify()const
  {
    return notifyFlag;
  }
  bool getWantNotifyMe()const
  {
    if(!wantNotifyMeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("wantNotifyMe");
    }
    return wantNotifyMe;
  }
  void setWantNotifyMe(bool argValue)
  {
    wantNotifyMe=argValue;
    wantNotifyMeFlag=true;
  }
  bool& getWantNotifyMeRef()
  {
    wantNotifyMeFlag=true;
    return wantNotifyMe;
  }
  bool hasWantNotifyMe()const
  {
    return wantNotifyMeFlag;
  }
  int8_t getInformTemplateId()const
  {
    if(!informTemplateIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("informTemplateId");
    }
    return informTemplateId;
  }
  void setInformTemplateId(int8_t argValue)
  {
    informTemplateId=argValue;
    informTemplateIdFlag=true;
  }
  int8_t& getInformTemplateIdRef()
  {
    informTemplateIdFlag=true;
    return informTemplateId;
  }
  bool hasInformTemplateId()const
  {
    return informTemplateIdFlag;
  }
  int8_t getNotifyTemplateId()const
  {
    if(!notifyTemplateIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("notifyTemplateId");
    }
    return notifyTemplateId;
  }
  void setNotifyTemplateId(int8_t argValue)
  {
    notifyTemplateId=argValue;
    notifyTemplateIdFlag=true;
  }
  int8_t& getNotifyTemplateIdRef()
  {
    notifyTemplateIdFlag=true;
    return notifyTemplateId;
  }
  bool hasNotifyTemplateId()const
  {
    return notifyTemplateIdFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    Response::serialize(ds);
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    if(eventMaskFlag)
    {
      ds.writeTag(eventMaskTag);
    ds.writeByteLV(eventMask); 
    }
    if(informFlag)
    {
      ds.writeTag(informTag);
    ds.writeBoolLV(inform); 
    }
    if(notifyFlag)
    {
      ds.writeTag(notifyTag);
    ds.writeBoolLV(notify); 
    }
    if(wantNotifyMeFlag)
    {
      ds.writeTag(wantNotifyMeTag);
    ds.writeBoolLV(wantNotifyMe); 
    }
    if(informTemplateIdFlag)
    {
      ds.writeTag(informTemplateIdTag);
    ds.writeByteLV(informTemplateId); 
    }
    if(notifyTemplateIdFlag)
    {
      ds.writeTag(notifyTemplateIdTag);
    ds.writeByteLV(notifyTemplateId); 
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
    //  throw protogen::framework::IncompatibleVersionException("GetProfileResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case eventMaskTag:
        {
          if(eventMaskFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("eventMask");
          }
          eventMask=ds.readByteLV();
          eventMaskFlag=true;
        }break;
        case informTag:
        {
          if(informFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("inform");
          }
          inform=ds.readBoolLV();
          informFlag=true;
        }break;
        case notifyTag:
        {
          if(notifyFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("notify");
          }
          notify=ds.readBoolLV();
          notifyFlag=true;
        }break;
        case wantNotifyMeTag:
        {
          if(wantNotifyMeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("wantNotifyMe");
          }
          wantNotifyMe=ds.readBoolLV();
          wantNotifyMeFlag=true;
        }break;
        case informTemplateIdTag:
        {
          if(informTemplateIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("informTemplateId");
          }
          informTemplateId=ds.readByteLV();
          informTemplateIdFlag=true;
        }break;
        case notifyTemplateIdTag:
        {
          if(notifyTemplateIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("notifyTemplateId");
          }
          notifyTemplateId=ds.readByteLV();
          notifyTemplateIdFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetProfileResp",tag);
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

  static const int32_t eventMaskTag=1;
  static const int32_t informTag=2;
  static const int32_t notifyTag=3;
  static const int32_t wantNotifyMeTag=4;
  static const int32_t informTemplateIdTag=5;
  static const int32_t notifyTemplateIdTag=6;

  int connId;

  int8_t eventMask;
  bool inform;
  bool notify;
  bool wantNotifyMe;
  int8_t informTemplateId;
  int8_t notifyTemplateId;

  bool eventMaskFlag;
  bool informFlag;
  bool notifyFlag;
  bool wantNotifyMeFlag;
  bool informTemplateIdFlag;
  bool notifyTemplateIdFlag;
};

}
}
}
}
#endif
