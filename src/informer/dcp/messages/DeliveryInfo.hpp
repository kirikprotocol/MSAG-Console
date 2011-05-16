#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYINFO_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYINFO_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryMode.hpp"


#ident "@(#) DeliveryInfo version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class DeliveryInfo{
public:
  DeliveryInfo()
  {
    Clear();
  }
  void Clear()
  {
    nameFlag=false;
    userIdFlag=false;
    priorityFlag=false;
    transactionModeFlag=false;
    startDateFlag=false;
    endDateFlag=false;
    activePeriodEndFlag=false;
    activePeriodStartFlag=false;
    activeWeekDaysFlag=false;
    activeWeekDays.clear();
    validityPeriodFlag=false;
    flashFlag=false;
    useDataSmFlag=false;
    deliveryModeFlag=false;
    ownerFlag=false;
    retryOnFailFlag=false;
    retryPolicyFlag=false;
    replaceMessageFlag=false;
    svcTypeFlag=false;
    userDataFlag=false;
    sourceAddressFlag=false;
    finalDlvRecordsFlag=false;
    finalMsgRecordsFlag=false;
    archivationPeriodFlag=false;
    messageTimeToLiveFlag=false;
  }
 

  static std::string messageGetName()
  {
    return "DeliveryInfo";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(nameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="name=";
      rv+=name;
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
    if(priorityFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="priority=";
      sprintf(buf,"%d",priority);
      rv+=buf;
    }
    if(transactionModeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="transactionMode=";
      rv+=transactionMode?"true":"false";
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
    if(activePeriodEndFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="activePeriodEnd=";
      rv+=activePeriodEnd;
    }
    if(activePeriodStartFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="activePeriodStart=";
      rv+=activePeriodStart;
    }
    if(activeWeekDaysFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="activeWeekDays=";
      rv+="[";
      bool first=true;
      for(std::vector<std::string>::const_iterator it=activeWeekDays.begin(),end=activeWeekDays.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=*it;
      }
      rv+="]";
    }
    if(validityPeriodFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="validityPeriod=";
      rv+=validityPeriod;
    }
    if(flashFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flash=";
      rv+=flash?"true":"false";
    }
    if(useDataSmFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="useDataSm=";
      rv+=useDataSm?"true":"false";
    }
    if(deliveryModeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="deliveryMode=";
      rv+=DeliveryMode::getNameByValue(deliveryMode);
    }
    if(ownerFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="owner=";
      rv+=owner;
    }
    if(retryOnFailFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="retryOnFail=";
      rv+=retryOnFail?"true":"false";
    }
    if(retryPolicyFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="retryPolicy=";
      rv+=retryPolicy;
    }
    if(replaceMessageFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="replaceMessage=";
      rv+=replaceMessage?"true":"false";
    }
    if(svcTypeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="svcType=";
      rv+=svcType;
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
    if(sourceAddressFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="sourceAddress=";
      rv+=sourceAddress;
    }
    if(finalDlvRecordsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="finalDlvRecords=";
      rv+=finalDlvRecords?"true":"false";
    }
    if(finalMsgRecordsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="finalMsgRecords=";
      rv+=finalMsgRecords?"true":"false";
    }
    if(archivationPeriodFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="archivationPeriod=";
      rv+=archivationPeriod;
    }
    if(messageTimeToLiveFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="messageTimeToLive=";
      rv+=messageTimeToLive;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(nameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(name);
    }
    if(userIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(userId);
    }
    if(priorityFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(priority);
    }
    if(transactionModeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(transactionMode);
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
    if(activePeriodEndFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(activePeriodEnd);
    }
    if(activePeriodStartFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(activePeriodStart);
    }
    if(activeWeekDaysFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(activeWeekDays);
    }
    if(validityPeriodFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(validityPeriod);
    }
    if(flashFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(flash);
    }
    if(useDataSmFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(useDataSm);
    }
    if(deliveryModeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(deliveryMode.getValue());
 
    }
    if(ownerFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(owner);
    }
    if(retryOnFailFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(retryOnFail);
    }
    if(retryPolicyFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(retryPolicy);
    }
    if(replaceMessageFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(replaceMessage);
    }
    if(svcTypeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(svcType);
    }
    if(userDataFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(userData);
    }
    if(sourceAddressFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(sourceAddress);
    }
    if(finalDlvRecordsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(finalDlvRecords);
    }
    if(finalMsgRecordsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(finalMsgRecords);
    }
    if(archivationPeriodFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(archivationPeriod);
    }
    if(messageTimeToLiveFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(messageTimeToLive);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
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
  int32_t getPriority()const
  {
    if(!priorityFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("priority");
    }
    return priority;
  }
  void setPriority(int32_t argValue)
  {
    priority=argValue;
    priorityFlag=true;
  }
  int32_t& getPriorityRef()
  {
    priorityFlag=true;
    return priority;
  }
  bool hasPriority()const
  {
    return priorityFlag;
  }
  bool getTransactionMode()const
  {
    if(!transactionModeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("transactionMode");
    }
    return transactionMode;
  }
  void setTransactionMode(bool argValue)
  {
    transactionMode=argValue;
    transactionModeFlag=true;
  }
  bool& getTransactionModeRef()
  {
    transactionModeFlag=true;
    return transactionMode;
  }
  bool hasTransactionMode()const
  {
    return transactionModeFlag;
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
  const std::string& getActivePeriodEnd()const
  {
    if(!activePeriodEndFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("activePeriodEnd");
    }
    return activePeriodEnd;
  }
  void setActivePeriodEnd(const std::string& argValue)
  {
    activePeriodEnd=argValue;
    activePeriodEndFlag=true;
  }
  std::string& getActivePeriodEndRef()
  {
    activePeriodEndFlag=true;
    return activePeriodEnd;
  }
  bool hasActivePeriodEnd()const
  {
    return activePeriodEndFlag;
  }
  const std::string& getActivePeriodStart()const
  {
    if(!activePeriodStartFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("activePeriodStart");
    }
    return activePeriodStart;
  }
  void setActivePeriodStart(const std::string& argValue)
  {
    activePeriodStart=argValue;
    activePeriodStartFlag=true;
  }
  std::string& getActivePeriodStartRef()
  {
    activePeriodStartFlag=true;
    return activePeriodStart;
  }
  bool hasActivePeriodStart()const
  {
    return activePeriodStartFlag;
  }
  const std::vector<std::string>& getActiveWeekDays()const
  {
    if(!activeWeekDaysFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("activeWeekDays");
    }
    return activeWeekDays;
  }
  void setActiveWeekDays(const std::vector<std::string>& argValue)
  {
    activeWeekDays=argValue;
    activeWeekDaysFlag=true;
  }
  std::vector<std::string>& getActiveWeekDaysRef()
  {
    activeWeekDaysFlag=true;
    return activeWeekDays;
  }
  bool hasActiveWeekDays()const
  {
    return activeWeekDaysFlag;
  }
  const std::string& getValidityPeriod()const
  {
    if(!validityPeriodFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("validityPeriod");
    }
    return validityPeriod;
  }
  void setValidityPeriod(const std::string& argValue)
  {
    validityPeriod=argValue;
    validityPeriodFlag=true;
  }
  std::string& getValidityPeriodRef()
  {
    validityPeriodFlag=true;
    return validityPeriod;
  }
  bool hasValidityPeriod()const
  {
    return validityPeriodFlag;
  }
  bool getFlash()const
  {
    if(!flashFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("flash");
    }
    return flash;
  }
  void setFlash(bool argValue)
  {
    flash=argValue;
    flashFlag=true;
  }
  bool& getFlashRef()
  {
    flashFlag=true;
    return flash;
  }
  bool hasFlash()const
  {
    return flashFlag;
  }
  bool getUseDataSm()const
  {
    if(!useDataSmFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("useDataSm");
    }
    return useDataSm;
  }
  void setUseDataSm(bool argValue)
  {
    useDataSm=argValue;
    useDataSmFlag=true;
  }
  bool& getUseDataSmRef()
  {
    useDataSmFlag=true;
    return useDataSm;
  }
  bool hasUseDataSm()const
  {
    return useDataSmFlag;
  }
  const DeliveryMode& getDeliveryMode()const
  {
    if(!deliveryModeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("deliveryMode");
    }
    return deliveryMode;
  }
  void setDeliveryMode(const DeliveryMode& argValue)
  {
    deliveryMode=argValue;
    deliveryModeFlag=true;
  }
  DeliveryMode& getDeliveryModeRef()
  {
    deliveryModeFlag=true;
    return deliveryMode;
  }
  bool hasDeliveryMode()const
  {
    return deliveryModeFlag;
  }
  const std::string& getOwner()const
  {
    if(!ownerFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("owner");
    }
    return owner;
  }
  void setOwner(const std::string& argValue)
  {
    owner=argValue;
    ownerFlag=true;
  }
  std::string& getOwnerRef()
  {
    ownerFlag=true;
    return owner;
  }
  bool hasOwner()const
  {
    return ownerFlag;
  }
  bool getRetryOnFail()const
  {
    if(!retryOnFailFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("retryOnFail");
    }
    return retryOnFail;
  }
  void setRetryOnFail(bool argValue)
  {
    retryOnFail=argValue;
    retryOnFailFlag=true;
  }
  bool& getRetryOnFailRef()
  {
    retryOnFailFlag=true;
    return retryOnFail;
  }
  bool hasRetryOnFail()const
  {
    return retryOnFailFlag;
  }
  const std::string& getRetryPolicy()const
  {
    if(!retryPolicyFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("retryPolicy");
    }
    return retryPolicy;
  }
  void setRetryPolicy(const std::string& argValue)
  {
    retryPolicy=argValue;
    retryPolicyFlag=true;
  }
  std::string& getRetryPolicyRef()
  {
    retryPolicyFlag=true;
    return retryPolicy;
  }
  bool hasRetryPolicy()const
  {
    return retryPolicyFlag;
  }
  bool getReplaceMessage()const
  {
    if(!replaceMessageFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("replaceMessage");
    }
    return replaceMessage;
  }
  void setReplaceMessage(bool argValue)
  {
    replaceMessage=argValue;
    replaceMessageFlag=true;
  }
  bool& getReplaceMessageRef()
  {
    replaceMessageFlag=true;
    return replaceMessage;
  }
  bool hasReplaceMessage()const
  {
    return replaceMessageFlag;
  }
  const std::string& getSvcType()const
  {
    if(!svcTypeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("svcType");
    }
    return svcType;
  }
  void setSvcType(const std::string& argValue)
  {
    svcType=argValue;
    svcTypeFlag=true;
  }
  std::string& getSvcTypeRef()
  {
    svcTypeFlag=true;
    return svcType;
  }
  bool hasSvcType()const
  {
    return svcTypeFlag;
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
  const std::string& getSourceAddress()const
  {
    if(!sourceAddressFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("sourceAddress");
    }
    return sourceAddress;
  }
  void setSourceAddress(const std::string& argValue)
  {
    sourceAddress=argValue;
    sourceAddressFlag=true;
  }
  std::string& getSourceAddressRef()
  {
    sourceAddressFlag=true;
    return sourceAddress;
  }
  bool hasSourceAddress()const
  {
    return sourceAddressFlag;
  }
  bool getFinalDlvRecords()const
  {
    if(!finalDlvRecordsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("finalDlvRecords");
    }
    return finalDlvRecords;
  }
  void setFinalDlvRecords(bool argValue)
  {
    finalDlvRecords=argValue;
    finalDlvRecordsFlag=true;
  }
  bool& getFinalDlvRecordsRef()
  {
    finalDlvRecordsFlag=true;
    return finalDlvRecords;
  }
  bool hasFinalDlvRecords()const
  {
    return finalDlvRecordsFlag;
  }
  bool getFinalMsgRecords()const
  {
    if(!finalMsgRecordsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("finalMsgRecords");
    }
    return finalMsgRecords;
  }
  void setFinalMsgRecords(bool argValue)
  {
    finalMsgRecords=argValue;
    finalMsgRecordsFlag=true;
  }
  bool& getFinalMsgRecordsRef()
  {
    finalMsgRecordsFlag=true;
    return finalMsgRecords;
  }
  bool hasFinalMsgRecords()const
  {
    return finalMsgRecordsFlag;
  }
  const std::string& getArchivationPeriod()const
  {
    if(!archivationPeriodFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("archivationPeriod");
    }
    return archivationPeriod;
  }
  void setArchivationPeriod(const std::string& argValue)
  {
    archivationPeriod=argValue;
    archivationPeriodFlag=true;
  }
  std::string& getArchivationPeriodRef()
  {
    archivationPeriodFlag=true;
    return archivationPeriod;
  }
  bool hasArchivationPeriod()const
  {
    return archivationPeriodFlag;
  }
  const std::string& getMessageTimeToLive()const
  {
    if(!messageTimeToLiveFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("messageTimeToLive");
    }
    return messageTimeToLive;
  }
  void setMessageTimeToLive(const std::string& argValue)
  {
    messageTimeToLive=argValue;
    messageTimeToLiveFlag=true;
  }
  std::string& getMessageTimeToLiveRef()
  {
    messageTimeToLiveFlag=true;
    return messageTimeToLive;
  }
  bool hasMessageTimeToLive()const
  {
    return messageTimeToLiveFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!nameFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("name");
    }
    if(!priorityFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("priority");
    }
    if(!transactionModeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("transactionMode");
    }
    if(!activePeriodEndFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("activePeriodEnd");
    }
    if(!activePeriodStartFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("activePeriodStart");
    }
    if(!activeWeekDaysFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("activeWeekDays");
    }
    if(!flashFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flash");
    }
    if(!useDataSmFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("useDataSm");
    }
    if(!deliveryModeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryMode");
    }
    if(!ownerFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("owner");
    }
    if(!retryOnFailFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("retryOnFail");
    }
    if(!replaceMessageFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("replaceMessage");
    }
    if(!sourceAddressFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("sourceAddress");
    }
    if(!finalDlvRecordsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("finalDlvRecords");
    }
    if(!finalMsgRecordsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("finalMsgRecords");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(nameTag);
    ds.writeStrLV(name); 
    ds.writeTag(priorityTag);
    ds.writeInt32LV(priority); 
    ds.writeTag(transactionModeTag);
    ds.writeBoolLV(transactionMode); 
    ds.writeTag(activePeriodEndTag);
    ds.writeStrLV(activePeriodEnd); 
    ds.writeTag(activePeriodStartTag);
    ds.writeStrLV(activePeriodStart); 
    ds.writeTag(activeWeekDaysTag);
    ds.writeLength(DataStream::fieldSize(activeWeekDays));
    for(std::vector<std::string>::const_iterator it=activeWeekDays.begin(),end=activeWeekDays.end();it!=end;++it)
    {
      ds.writeStr(*it);
          }
    ds.writeTag(flashTag);
    ds.writeBoolLV(flash); 
    ds.writeTag(useDataSmTag);
    ds.writeBoolLV(useDataSm); 
    ds.writeTag(deliveryModeTag);
    ds.writeByteLV(deliveryMode.getValue());
 
    ds.writeTag(ownerTag);
    ds.writeStrLV(owner); 
    ds.writeTag(retryOnFailTag);
    ds.writeBoolLV(retryOnFail); 
    ds.writeTag(replaceMessageTag);
    ds.writeBoolLV(replaceMessage); 
    ds.writeTag(sourceAddressTag);
    ds.writeStrLV(sourceAddress); 
    ds.writeTag(finalDlvRecordsTag);
    ds.writeBoolLV(finalDlvRecords); 
    ds.writeTag(finalMsgRecordsTag);
    ds.writeBoolLV(finalMsgRecords); 
    if(userIdFlag)
    {
      ds.writeTag(userIdTag);
    ds.writeStrLV(userId); 
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
    if(validityPeriodFlag)
    {
      ds.writeTag(validityPeriodTag);
    ds.writeStrLV(validityPeriod); 
    }
    if(retryPolicyFlag)
    {
      ds.writeTag(retryPolicyTag);
    ds.writeStrLV(retryPolicy); 
    }
    if(svcTypeFlag)
    {
      ds.writeTag(svcTypeTag);
    ds.writeStrLV(svcType); 
    }
    if(userDataFlag)
    {
      ds.writeTag(userDataTag);
    ds.writeStrLV(userData); 
    }
    if(archivationPeriodFlag)
    {
      ds.writeTag(archivationPeriodTag);
    ds.writeStrLV(archivationPeriod); 
    }
    if(messageTimeToLiveFlag)
    {
      ds.writeTag(messageTimeToLiveTag);
    ds.writeStrLV(messageTimeToLive); 
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
    //  throw protogen::framework::IncompatibleVersionException("DeliveryInfo");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case nameTag:
        {
          if(nameFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("name");
          }
          name=ds.readStrLV();
          nameFlag=true;
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
        case priorityTag:
        {
          if(priorityFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("priority");
          }
          priority=ds.readInt32LV();
          priorityFlag=true;
        }break;
        case transactionModeTag:
        {
          if(transactionModeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("transactionMode");
          }
          transactionMode=ds.readBoolLV();
          transactionModeFlag=true;
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
        case activePeriodEndTag:
        {
          if(activePeriodEndFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("activePeriodEnd");
          }
          activePeriodEnd=ds.readStrLV();
          activePeriodEndFlag=true;
        }break;
        case activePeriodStartTag:
        {
          if(activePeriodStartFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("activePeriodStart");
          }
          activePeriodStart=ds.readStrLV();
          activePeriodStartFlag=true;
        }break;
        case activeWeekDaysTag:
        {
          if(activeWeekDaysFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("activeWeekDays");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            activeWeekDays.push_back(ds.readStr());
            rd+=DataStream::fieldSize(activeWeekDays.back());
            rd+=DataStream::lengthTypeSize;
          }
          activeWeekDaysFlag=true;
        }break;
        case validityPeriodTag:
        {
          if(validityPeriodFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("validityPeriod");
          }
          validityPeriod=ds.readStrLV();
          validityPeriodFlag=true;
        }break;
        case flashTag:
        {
          if(flashFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("flash");
          }
          flash=ds.readBoolLV();
          flashFlag=true;
        }break;
        case useDataSmTag:
        {
          if(useDataSmFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("useDataSm");
          }
          useDataSm=ds.readBoolLV();
          useDataSmFlag=true;
        }break;
        case deliveryModeTag:
        {
          if(deliveryModeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("deliveryMode");
          }
          deliveryMode=ds.readByteLV();
          deliveryModeFlag=true;
        }break;
        case ownerTag:
        {
          if(ownerFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("owner");
          }
          owner=ds.readStrLV();
          ownerFlag=true;
        }break;
        case retryOnFailTag:
        {
          if(retryOnFailFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("retryOnFail");
          }
          retryOnFail=ds.readBoolLV();
          retryOnFailFlag=true;
        }break;
        case retryPolicyTag:
        {
          if(retryPolicyFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("retryPolicy");
          }
          retryPolicy=ds.readStrLV();
          retryPolicyFlag=true;
        }break;
        case replaceMessageTag:
        {
          if(replaceMessageFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("replaceMessage");
          }
          replaceMessage=ds.readBoolLV();
          replaceMessageFlag=true;
        }break;
        case svcTypeTag:
        {
          if(svcTypeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("svcType");
          }
          svcType=ds.readStrLV();
          svcTypeFlag=true;
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
        case sourceAddressTag:
        {
          if(sourceAddressFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("sourceAddress");
          }
          sourceAddress=ds.readStrLV();
          sourceAddressFlag=true;
        }break;
        case finalDlvRecordsTag:
        {
          if(finalDlvRecordsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("finalDlvRecords");
          }
          finalDlvRecords=ds.readBoolLV();
          finalDlvRecordsFlag=true;
        }break;
        case finalMsgRecordsTag:
        {
          if(finalMsgRecordsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("finalMsgRecords");
          }
          finalMsgRecords=ds.readBoolLV();
          finalMsgRecordsFlag=true;
        }break;
        case archivationPeriodTag:
        {
          if(archivationPeriodFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("archivationPeriod");
          }
          archivationPeriod=ds.readStrLV();
          archivationPeriodFlag=true;
        }break;
        case messageTimeToLiveTag:
        {
          if(messageTimeToLiveFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("messageTimeToLive");
          }
          messageTimeToLive=ds.readStrLV();
          messageTimeToLiveFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DeliveryInfo",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!nameFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("name");
    }
    if(!priorityFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("priority");
    }
    if(!transactionModeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("transactionMode");
    }
    if(!activePeriodEndFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("activePeriodEnd");
    }
    if(!activePeriodStartFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("activePeriodStart");
    }
    if(!activeWeekDaysFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("activeWeekDays");
    }
    if(!flashFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flash");
    }
    if(!useDataSmFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("useDataSm");
    }
    if(!deliveryModeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryMode");
    }
    if(!ownerFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("owner");
    }
    if(!retryOnFailFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("retryOnFail");
    }
    if(!replaceMessageFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("replaceMessage");
    }
    if(!sourceAddressFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("sourceAddress");
    }
    if(!finalDlvRecordsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("finalDlvRecords");
    }
    if(!finalMsgRecordsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("finalMsgRecords");
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

  static const int32_t nameTag=1;
  static const int32_t userIdTag=2;
  static const int32_t priorityTag=3;
  static const int32_t transactionModeTag=4;
  static const int32_t startDateTag=5;
  static const int32_t endDateTag=6;
  static const int32_t activePeriodEndTag=7;
  static const int32_t activePeriodStartTag=8;
  static const int32_t activeWeekDaysTag=9;
  static const int32_t validityPeriodTag=11;
  static const int32_t flashTag=12;
  static const int32_t useDataSmTag=16;
  static const int32_t deliveryModeTag=17;
  static const int32_t ownerTag=18;
  static const int32_t retryOnFailTag=19;
  static const int32_t retryPolicyTag=20;
  static const int32_t replaceMessageTag=21;
  static const int32_t svcTypeTag=22;
  static const int32_t userDataTag=23;
  static const int32_t sourceAddressTag=24;
  static const int32_t finalDlvRecordsTag=25;
  static const int32_t finalMsgRecordsTag=26;
  static const int32_t archivationPeriodTag=27;
  static const int32_t messageTimeToLiveTag=28;

  int connId;

  std::string name;
  std::string userId;
  int32_t priority;
  bool transactionMode;
  std::string startDate;
  std::string endDate;
  std::string activePeriodEnd;
  std::string activePeriodStart;
  std::vector<std::string> activeWeekDays;
  std::string validityPeriod;
  bool flash;
  bool useDataSm;
  DeliveryMode deliveryMode;
  std::string owner;
  bool retryOnFail;
  std::string retryPolicy;
  bool replaceMessage;
  std::string svcType;
  std::string userData;
  std::string sourceAddress;
  bool finalDlvRecords;
  bool finalMsgRecords;
  std::string archivationPeriod;
  std::string messageTimeToLive;

  bool nameFlag;
  bool userIdFlag;
  bool priorityFlag;
  bool transactionModeFlag;
  bool startDateFlag;
  bool endDateFlag;
  bool activePeriodEndFlag;
  bool activePeriodStartFlag;
  bool activeWeekDaysFlag;
  bool validityPeriodFlag;
  bool flashFlag;
  bool useDataSmFlag;
  bool deliveryModeFlag;
  bool ownerFlag;
  bool retryOnFailFlag;
  bool retryPolicyFlag;
  bool replaceMessageFlag;
  bool svcTypeFlag;
  bool userDataFlag;
  bool sourceAddressFlag;
  bool finalDlvRecordsFlag;
  bool finalMsgRecordsFlag;
  bool archivationPeriodFlag;
  bool messageTimeToLiveFlag;
};

}
}
}
}
#endif
