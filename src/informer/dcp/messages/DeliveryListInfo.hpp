#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYLISTINFO_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYLISTINFO_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryStatus.hpp"


#ident "@(#) DeliveryListInfo version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class DeliveryListInfo{
public:
  DeliveryListInfo()
  {
    Clear();
  }
  void Clear()
  {
    deliveryIdFlag=false;
    userIdFlag=false;
    nameFlag=false;
    statusFlag=false;
    startDateFlag=false;
    endDateFlag=false;
    activityPeriodStartFlag=false;
    activityPeriodEndFlag=false;
    userDataFlag=false;
    creationDateFlag=false;
    boundToLocalTimeFlag=false;
  }
 

  static std::string messageGetName()
  {
    return "DeliveryListInfo";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(deliveryIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="deliveryId=";
      sprintf(buf,"%d",deliveryId);
      rv+=buf;
    }
    if(userIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="userId=";
      rv+=userId;
    }
    if(nameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="name=";
      rv+=name;
    }
    if(statusFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="status=";
      rv+=DeliveryStatus::getNameByValue(status);
    }
    if(startDateFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="startDate=";
      rv+=startDate;
    }
    if(endDateFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="endDate=";
      rv+=endDate;
    }
    if(activityPeriodStartFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="activityPeriodStart=";
      rv+=activityPeriodStart;
    }
    if(activityPeriodEndFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="activityPeriodEnd=";
      rv+=activityPeriodEnd;
    }
    if(userDataFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="userData=";
      rv+=userData;
    }
    if(creationDateFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="creationDate=";
      rv+=creationDate;
    }
    if(boundToLocalTimeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="boundToLocalTime=";
      rv+=boundToLocalTime?"true":"false";
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(deliveryIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(deliveryId);
    }
    if(userIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(userId);
    }
    if(nameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(name);
    }
    if(statusFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(status.getValue());
 
    }
    if(startDateFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(startDate);
    }
    if(endDateFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(endDate);
    }
    if(activityPeriodStartFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(activityPeriodStart);
    }
    if(activityPeriodEndFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(activityPeriodEnd);
    }
    if(userDataFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(userData);
    }
    if(creationDateFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(creationDate);
    }
    if(boundToLocalTimeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(boundToLocalTime);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getDeliveryId()const
  {
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("deliveryId");
    }
    return deliveryId;
  }
  void setDeliveryId(int32_t argValue)
  {
    deliveryId=argValue;
    deliveryIdFlag=true;
  }
  int32_t& getDeliveryIdRef()
  {
    deliveryIdFlag=true;
    return deliveryId;
  }
  bool hasDeliveryId()const
  {
    return deliveryIdFlag;
  }
  const std::string& getUserId()const
  {
    if(!userIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("userId");
    }
    return userId;
  }
  void setUserId(const std::string& argValue)
  {
    userId=argValue;
    userIdFlag=true;
  }
  std::string& getUserIdRef()
  {
    userIdFlag=true;
    return userId;
  }
  bool hasUserId()const
  {
    return userIdFlag;
  }
  const std::string& getName()const
  {
    if(!nameFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("name");
    }
    return name;
  }
  void setName(const std::string& argValue)
  {
    name=argValue;
    nameFlag=true;
  }
  std::string& getNameRef()
  {
    nameFlag=true;
    return name;
  }
  bool hasName()const
  {
    return nameFlag;
  }
  const DeliveryStatus& getStatus()const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("status");
    }
    return status;
  }
  void setStatus(const DeliveryStatus& argValue)
  {
    status=argValue;
    statusFlag=true;
  }
  DeliveryStatus& getStatusRef()
  {
    statusFlag=true;
    return status;
  }
  bool hasStatus()const
  {
    return statusFlag;
  }
  const std::string& getStartDate()const
  {
    if(!startDateFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("startDate");
    }
    return startDate;
  }
  void setStartDate(const std::string& argValue)
  {
    startDate=argValue;
    startDateFlag=true;
  }
  std::string& getStartDateRef()
  {
    startDateFlag=true;
    return startDate;
  }
  bool hasStartDate()const
  {
    return startDateFlag;
  }
  const std::string& getEndDate()const
  {
    if(!endDateFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("endDate");
    }
    return endDate;
  }
  void setEndDate(const std::string& argValue)
  {
    endDate=argValue;
    endDateFlag=true;
  }
  std::string& getEndDateRef()
  {
    endDateFlag=true;
    return endDate;
  }
  bool hasEndDate()const
  {
    return endDateFlag;
  }
  const std::string& getActivityPeriodStart()const
  {
    if(!activityPeriodStartFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("activityPeriodStart");
    }
    return activityPeriodStart;
  }
  void setActivityPeriodStart(const std::string& argValue)
  {
    activityPeriodStart=argValue;
    activityPeriodStartFlag=true;
  }
  std::string& getActivityPeriodStartRef()
  {
    activityPeriodStartFlag=true;
    return activityPeriodStart;
  }
  bool hasActivityPeriodStart()const
  {
    return activityPeriodStartFlag;
  }
  const std::string& getActivityPeriodEnd()const
  {
    if(!activityPeriodEndFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("activityPeriodEnd");
    }
    return activityPeriodEnd;
  }
  void setActivityPeriodEnd(const std::string& argValue)
  {
    activityPeriodEnd=argValue;
    activityPeriodEndFlag=true;
  }
  std::string& getActivityPeriodEndRef()
  {
    activityPeriodEndFlag=true;
    return activityPeriodEnd;
  }
  bool hasActivityPeriodEnd()const
  {
    return activityPeriodEndFlag;
  }
  const std::string& getUserData()const
  {
    if(!userDataFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("userData");
    }
    return userData;
  }
  void setUserData(const std::string& argValue)
  {
    userData=argValue;
    userDataFlag=true;
  }
  std::string& getUserDataRef()
  {
    userDataFlag=true;
    return userData;
  }
  bool hasUserData()const
  {
    return userDataFlag;
  }
  const std::string& getCreationDate()const
  {
    if(!creationDateFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("creationDate");
    }
    return creationDate;
  }
  void setCreationDate(const std::string& argValue)
  {
    creationDate=argValue;
    creationDateFlag=true;
  }
  std::string& getCreationDateRef()
  {
    creationDateFlag=true;
    return creationDate;
  }
  bool hasCreationDate()const
  {
    return creationDateFlag;
  }
  bool getBoundToLocalTime()const
  {
    if(!boundToLocalTimeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("boundToLocalTime");
    }
    return boundToLocalTime;
  }
  void setBoundToLocalTime(bool argValue)
  {
    boundToLocalTime=argValue;
    boundToLocalTimeFlag=true;
  }
  bool& getBoundToLocalTimeRef()
  {
    boundToLocalTimeFlag=true;
    return boundToLocalTime;
  }
  bool hasBoundToLocalTime()const
  {
    return boundToLocalTimeFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryId");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(deliveryIdTag);
    ds.writeInt32LV(deliveryId); 
    if(userIdFlag)
    {
      ds.writeTag(userIdTag);
    ds.writeStrLV(userId); 
    }
    if(nameFlag)
    {
      ds.writeTag(nameTag);
    ds.writeStrLV(name); 
    }
    if(statusFlag)
    {
      ds.writeTag(statusTag);
    ds.writeByteLV(status.getValue());
 
    }
    if(startDateFlag)
    {
      ds.writeTag(startDateTag);
    ds.writeStrLV(startDate); 
    }
    if(endDateFlag)
    {
      ds.writeTag(endDateTag);
    ds.writeStrLV(endDate); 
    }
    if(activityPeriodStartFlag)
    {
      ds.writeTag(activityPeriodStartTag);
    ds.writeStrLV(activityPeriodStart); 
    }
    if(activityPeriodEndFlag)
    {
      ds.writeTag(activityPeriodEndTag);
    ds.writeStrLV(activityPeriodEnd); 
    }
    if(userDataFlag)
    {
      ds.writeTag(userDataTag);
    ds.writeStrLV(userData); 
    }
    if(creationDateFlag)
    {
      ds.writeTag(creationDateTag);
    ds.writeStrLV(creationDate); 
    }
    if(boundToLocalTimeFlag)
    {
      ds.writeTag(boundToLocalTimeTag);
    ds.writeBoolLV(boundToLocalTime); 
    }
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
    //  throw protogen::framework::IncompatibleVersionException("DeliveryListInfo");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case deliveryIdTag:
        {
          if(deliveryIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("deliveryId");
          }
          deliveryId=ds.readInt32LV();
          deliveryIdFlag=true;
        }break;
        case userIdTag:
        {
          if(userIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("userId");
          }
          userId=ds.readStrLV();
          userIdFlag=true;
        }break;
        case nameTag:
        {
          if(nameFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("name");
          }
          name=ds.readStrLV();
          nameFlag=true;
        }break;
        case statusTag:
        {
          if(statusFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("status");
          }
          status=ds.readByteLV();
          statusFlag=true;
        }break;
        case startDateTag:
        {
          if(startDateFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("startDate");
          }
          startDate=ds.readStrLV();
          startDateFlag=true;
        }break;
        case endDateTag:
        {
          if(endDateFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("endDate");
          }
          endDate=ds.readStrLV();
          endDateFlag=true;
        }break;
        case activityPeriodStartTag:
        {
          if(activityPeriodStartFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("activityPeriodStart");
          }
          activityPeriodStart=ds.readStrLV();
          activityPeriodStartFlag=true;
        }break;
        case activityPeriodEndTag:
        {
          if(activityPeriodEndFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("activityPeriodEnd");
          }
          activityPeriodEnd=ds.readStrLV();
          activityPeriodEndFlag=true;
        }break;
        case userDataTag:
        {
          if(userDataFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("userData");
          }
          userData=ds.readStrLV();
          userDataFlag=true;
        }break;
        case creationDateTag:
        {
          if(creationDateFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("creationDate");
          }
          creationDate=ds.readStrLV();
          creationDateFlag=true;
        }break;
        case boundToLocalTimeTag:
        {
          if(boundToLocalTimeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("boundToLocalTime");
          }
          boundToLocalTime=ds.readBoolLV();
          boundToLocalTimeFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DeliveryListInfo",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryId");
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

  static const int32_t deliveryIdTag=1;
  static const int32_t userIdTag=2;
  static const int32_t nameTag=3;
  static const int32_t statusTag=4;
  static const int32_t startDateTag=5;
  static const int32_t endDateTag=6;
  static const int32_t activityPeriodStartTag=7;
  static const int32_t activityPeriodEndTag=8;
  static const int32_t userDataTag=9;
  static const int32_t creationDateTag=10;
  static const int32_t boundToLocalTimeTag=11;

  int connId;

  int32_t deliveryId;
  std::string userId;
  std::string name;
  DeliveryStatus status;
  std::string startDate;
  std::string endDate;
  std::string activityPeriodStart;
  std::string activityPeriodEnd;
  std::string userData;
  std::string creationDate;
  bool boundToLocalTime;

  bool deliveryIdFlag;
  bool userIdFlag;
  bool nameFlag;
  bool statusFlag;
  bool startDateFlag;
  bool endDateFlag;
  bool activityPeriodStartFlag;
  bool activityPeriodEndFlag;
  bool userDataFlag;
  bool creationDateFlag;
  bool boundToLocalTimeFlag;
};

}
}
}
}
#endif
