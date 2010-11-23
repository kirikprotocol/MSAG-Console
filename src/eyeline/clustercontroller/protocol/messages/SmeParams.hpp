#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMEPARAMS_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMEPARAMS_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "SmeBindMode.hpp"


#ident "@(#) SmeParams version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class SmeParams{
public:
  SmeParams()
  {
    Clear();
  }
  void Clear()
  {
    idFlag=false;
    priorityFlag=false;
    systemTypeFlag=false;
    passwordFlag=false;
    addrRangeFlag=false;
    smeNFlag=false;
    wantAliasFlag=false;
    disabledFlag=false;
    timeoutFlag=false;
    receiptSchemeFlag=false;
    modeFlag=false;
    procLimitFlag=false;
    schedLimitFlag=false;
    accessMaskFlag=false;
    flagCarryOrgDescriptorFlag=false;
    flagCarryOrgAbonentInfoFlag=false;
    flagCarrySccpInfoFlag=false;
    flagFillExtraDescriptorFlag=false;
    flagForceReceiptToSmeFlag=false;
    flagForceGsmDatacodingFlag=false;
    flagSmppPlusFlag=false;
  }
 

  static std::string messageGetName()
  {
    return "SmeParams";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(idFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="id=";
      rv+=id;
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
    if(systemTypeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="systemType=";
      rv+=systemType;
    }
    if(passwordFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="password=";
      rv+=password;
    }
    if(addrRangeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="addrRange=";
      rv+=addrRange;
    }
    if(smeNFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="smeN=";
      sprintf(buf,"%d",smeN);
      rv+=buf;
    }
    if(wantAliasFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="wantAlias=";
      rv+=wantAlias?"true":"false";
    }
    if(disabledFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="disabled=";
      rv+=disabled?"true":"false";
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
    if(receiptSchemeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="receiptScheme=";
      rv+=receiptScheme;
    }
    if(modeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="mode=";
      rv+=SmeBindMode::getNameByValue(mode);
    }
    if(procLimitFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="procLimit=";
      sprintf(buf,"%d",procLimit);
      rv+=buf;
    }
    if(schedLimitFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="schedLimit=";
      sprintf(buf,"%d",schedLimit);
      rv+=buf;
    }
    if(accessMaskFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="accessMask=";
      sprintf(buf,"%d",accessMask);
      rv+=buf;
    }
    if(flagCarryOrgDescriptorFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flagCarryOrgDescriptor=";
      rv+=flagCarryOrgDescriptor?"true":"false";
    }
    if(flagCarryOrgAbonentInfoFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flagCarryOrgAbonentInfo=";
      rv+=flagCarryOrgAbonentInfo?"true":"false";
    }
    if(flagCarrySccpInfoFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flagCarrySccpInfo=";
      rv+=flagCarrySccpInfo?"true":"false";
    }
    if(flagFillExtraDescriptorFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flagFillExtraDescriptor=";
      rv+=flagFillExtraDescriptor?"true":"false";
    }
    if(flagForceReceiptToSmeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flagForceReceiptToSme=";
      rv+=flagForceReceiptToSme?"true":"false";
    }
    if(flagForceGsmDatacodingFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flagForceGsmDatacoding=";
      rv+=flagForceGsmDatacoding?"true":"false";
    }
    if(flagSmppPlusFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flagSmppPlus=";
      rv+=flagSmppPlus?"true":"false";
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(idFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(id);
    }
    if(priorityFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(priority);
    }
    if(systemTypeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(systemType);
    }
    if(passwordFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(password);
    }
    if(addrRangeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(addrRange);
    }
    if(smeNFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(smeN);
    }
    if(wantAliasFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(wantAlias);
    }
    if(disabledFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(disabled);
    }
    if(timeoutFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(timeout);
    }
    if(receiptSchemeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(receiptScheme);
    }
    if(modeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(mode.getValue());
 
    }
    if(procLimitFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(procLimit);
    }
    if(schedLimitFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(schedLimit);
    }
    if(accessMaskFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(accessMask);
    }
    if(flagCarryOrgDescriptorFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(flagCarryOrgDescriptor);
    }
    if(flagCarryOrgAbonentInfoFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(flagCarryOrgAbonentInfo);
    }
    if(flagCarrySccpInfoFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(flagCarrySccpInfo);
    }
    if(flagFillExtraDescriptorFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(flagFillExtraDescriptor);
    }
    if(flagForceReceiptToSmeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(flagForceReceiptToSme);
    }
    if(flagForceGsmDatacodingFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(flagForceGsmDatacoding);
    }
    if(flagSmppPlusFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(flagSmppPlus);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getId()const
  {
    if(!idFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("id");
    }
    return id;
  }
  void setId(const std::string& argValue)
  {
    id=argValue;
    idFlag=true;
  }
  std::string& getIdRef()
  {
    idFlag=true;
    return id;
  }
  bool hasId()const
  {
    return idFlag;
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
  const std::string& getSystemType()const
  {
    if(!systemTypeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("systemType");
    }
    return systemType;
  }
  void setSystemType(const std::string& argValue)
  {
    systemType=argValue;
    systemTypeFlag=true;
  }
  std::string& getSystemTypeRef()
  {
    systemTypeFlag=true;
    return systemType;
  }
  bool hasSystemType()const
  {
    return systemTypeFlag;
  }
  const std::string& getPassword()const
  {
    if(!passwordFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("password");
    }
    return password;
  }
  void setPassword(const std::string& argValue)
  {
    password=argValue;
    passwordFlag=true;
  }
  std::string& getPasswordRef()
  {
    passwordFlag=true;
    return password;
  }
  bool hasPassword()const
  {
    return passwordFlag;
  }
  const std::string& getAddrRange()const
  {
    if(!addrRangeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("addrRange");
    }
    return addrRange;
  }
  void setAddrRange(const std::string& argValue)
  {
    addrRange=argValue;
    addrRangeFlag=true;
  }
  std::string& getAddrRangeRef()
  {
    addrRangeFlag=true;
    return addrRange;
  }
  bool hasAddrRange()const
  {
    return addrRangeFlag;
  }
  int32_t getSmeN()const
  {
    if(!smeNFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("smeN");
    }
    return smeN;
  }
  void setSmeN(int32_t argValue)
  {
    smeN=argValue;
    smeNFlag=true;
  }
  int32_t& getSmeNRef()
  {
    smeNFlag=true;
    return smeN;
  }
  bool hasSmeN()const
  {
    return smeNFlag;
  }
  bool getWantAlias()const
  {
    if(!wantAliasFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("wantAlias");
    }
    return wantAlias;
  }
  void setWantAlias(bool argValue)
  {
    wantAlias=argValue;
    wantAliasFlag=true;
  }
  bool& getWantAliasRef()
  {
    wantAliasFlag=true;
    return wantAlias;
  }
  bool hasWantAlias()const
  {
    return wantAliasFlag;
  }
  bool getDisabled()const
  {
    if(!disabledFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("disabled");
    }
    return disabled;
  }
  void setDisabled(bool argValue)
  {
    disabled=argValue;
    disabledFlag=true;
  }
  bool& getDisabledRef()
  {
    disabledFlag=true;
    return disabled;
  }
  bool hasDisabled()const
  {
    return disabledFlag;
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
  const std::string& getReceiptScheme()const
  {
    if(!receiptSchemeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("receiptScheme");
    }
    return receiptScheme;
  }
  void setReceiptScheme(const std::string& argValue)
  {
    receiptScheme=argValue;
    receiptSchemeFlag=true;
  }
  std::string& getReceiptSchemeRef()
  {
    receiptSchemeFlag=true;
    return receiptScheme;
  }
  bool hasReceiptScheme()const
  {
    return receiptSchemeFlag;
  }
  const SmeBindMode& getMode()const
  {
    if(!modeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("mode");
    }
    return mode;
  }
  void setMode(const SmeBindMode& argValue)
  {
    mode=argValue;
    modeFlag=true;
  }
  SmeBindMode& getModeRef()
  {
    modeFlag=true;
    return mode;
  }
  bool hasMode()const
  {
    return modeFlag;
  }
  int32_t getProcLimit()const
  {
    if(!procLimitFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("procLimit");
    }
    return procLimit;
  }
  void setProcLimit(int32_t argValue)
  {
    procLimit=argValue;
    procLimitFlag=true;
  }
  int32_t& getProcLimitRef()
  {
    procLimitFlag=true;
    return procLimit;
  }
  bool hasProcLimit()const
  {
    return procLimitFlag;
  }
  int32_t getSchedLimit()const
  {
    if(!schedLimitFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("schedLimit");
    }
    return schedLimit;
  }
  void setSchedLimit(int32_t argValue)
  {
    schedLimit=argValue;
    schedLimitFlag=true;
  }
  int32_t& getSchedLimitRef()
  {
    schedLimitFlag=true;
    return schedLimit;
  }
  bool hasSchedLimit()const
  {
    return schedLimitFlag;
  }
  int32_t getAccessMask()const
  {
    if(!accessMaskFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("accessMask");
    }
    return accessMask;
  }
  void setAccessMask(int32_t argValue)
  {
    accessMask=argValue;
    accessMaskFlag=true;
  }
  int32_t& getAccessMaskRef()
  {
    accessMaskFlag=true;
    return accessMask;
  }
  bool hasAccessMask()const
  {
    return accessMaskFlag;
  }
  bool getFlagCarryOrgDescriptor()const
  {
    if(!flagCarryOrgDescriptorFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("flagCarryOrgDescriptor");
    }
    return flagCarryOrgDescriptor;
  }
  void setFlagCarryOrgDescriptor(bool argValue)
  {
    flagCarryOrgDescriptor=argValue;
    flagCarryOrgDescriptorFlag=true;
  }
  bool& getFlagCarryOrgDescriptorRef()
  {
    flagCarryOrgDescriptorFlag=true;
    return flagCarryOrgDescriptor;
  }
  bool hasFlagCarryOrgDescriptor()const
  {
    return flagCarryOrgDescriptorFlag;
  }
  bool getFlagCarryOrgAbonentInfo()const
  {
    if(!flagCarryOrgAbonentInfoFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("flagCarryOrgAbonentInfo");
    }
    return flagCarryOrgAbonentInfo;
  }
  void setFlagCarryOrgAbonentInfo(bool argValue)
  {
    flagCarryOrgAbonentInfo=argValue;
    flagCarryOrgAbonentInfoFlag=true;
  }
  bool& getFlagCarryOrgAbonentInfoRef()
  {
    flagCarryOrgAbonentInfoFlag=true;
    return flagCarryOrgAbonentInfo;
  }
  bool hasFlagCarryOrgAbonentInfo()const
  {
    return flagCarryOrgAbonentInfoFlag;
  }
  bool getFlagCarrySccpInfo()const
  {
    if(!flagCarrySccpInfoFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("flagCarrySccpInfo");
    }
    return flagCarrySccpInfo;
  }
  void setFlagCarrySccpInfo(bool argValue)
  {
    flagCarrySccpInfo=argValue;
    flagCarrySccpInfoFlag=true;
  }
  bool& getFlagCarrySccpInfoRef()
  {
    flagCarrySccpInfoFlag=true;
    return flagCarrySccpInfo;
  }
  bool hasFlagCarrySccpInfo()const
  {
    return flagCarrySccpInfoFlag;
  }
  bool getFlagFillExtraDescriptor()const
  {
    if(!flagFillExtraDescriptorFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("flagFillExtraDescriptor");
    }
    return flagFillExtraDescriptor;
  }
  void setFlagFillExtraDescriptor(bool argValue)
  {
    flagFillExtraDescriptor=argValue;
    flagFillExtraDescriptorFlag=true;
  }
  bool& getFlagFillExtraDescriptorRef()
  {
    flagFillExtraDescriptorFlag=true;
    return flagFillExtraDescriptor;
  }
  bool hasFlagFillExtraDescriptor()const
  {
    return flagFillExtraDescriptorFlag;
  }
  bool getFlagForceReceiptToSme()const
  {
    if(!flagForceReceiptToSmeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("flagForceReceiptToSme");
    }
    return flagForceReceiptToSme;
  }
  void setFlagForceReceiptToSme(bool argValue)
  {
    flagForceReceiptToSme=argValue;
    flagForceReceiptToSmeFlag=true;
  }
  bool& getFlagForceReceiptToSmeRef()
  {
    flagForceReceiptToSmeFlag=true;
    return flagForceReceiptToSme;
  }
  bool hasFlagForceReceiptToSme()const
  {
    return flagForceReceiptToSmeFlag;
  }
  bool getFlagForceGsmDatacoding()const
  {
    if(!flagForceGsmDatacodingFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("flagForceGsmDatacoding");
    }
    return flagForceGsmDatacoding;
  }
  void setFlagForceGsmDatacoding(bool argValue)
  {
    flagForceGsmDatacoding=argValue;
    flagForceGsmDatacodingFlag=true;
  }
  bool& getFlagForceGsmDatacodingRef()
  {
    flagForceGsmDatacodingFlag=true;
    return flagForceGsmDatacoding;
  }
  bool hasFlagForceGsmDatacoding()const
  {
    return flagForceGsmDatacodingFlag;
  }
  bool getFlagSmppPlus()const
  {
    if(!flagSmppPlusFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("flagSmppPlus");
    }
    return flagSmppPlus;
  }
  void setFlagSmppPlus(bool argValue)
  {
    flagSmppPlus=argValue;
    flagSmppPlusFlag=true;
  }
  bool& getFlagSmppPlusRef()
  {
    flagSmppPlusFlag=true;
    return flagSmppPlus;
  }
  bool hasFlagSmppPlus()const
  {
    return flagSmppPlusFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!idFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!priorityFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("priority");
    }
    if(!systemTypeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("systemType");
    }
    if(!passwordFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("password");
    }
    if(!addrRangeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("addrRange");
    }
    if(!smeNFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("smeN");
    }
    if(!wantAliasFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("wantAlias");
    }
    if(!disabledFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("disabled");
    }
    if(!timeoutFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("timeout");
    }
    if(!receiptSchemeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("receiptScheme");
    }
    if(!modeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("mode");
    }
    if(!procLimitFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("procLimit");
    }
    if(!schedLimitFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("schedLimit");
    }
    if(!accessMaskFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("accessMask");
    }
    if(!flagCarryOrgDescriptorFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagCarryOrgDescriptor");
    }
    if(!flagCarryOrgAbonentInfoFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagCarryOrgAbonentInfo");
    }
    if(!flagCarrySccpInfoFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagCarrySccpInfo");
    }
    if(!flagFillExtraDescriptorFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagFillExtraDescriptor");
    }
    if(!flagForceReceiptToSmeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagForceReceiptToSme");
    }
    if(!flagForceGsmDatacodingFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagForceGsmDatacoding");
    }
    if(!flagSmppPlusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagSmppPlus");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(idTag);
    ds.writeStrLV(id); 
    ds.writeTag(priorityTag);
    ds.writeInt32LV(priority); 
    ds.writeTag(systemTypeTag);
    ds.writeStrLV(systemType); 
    ds.writeTag(passwordTag);
    ds.writeStrLV(password); 
    ds.writeTag(addrRangeTag);
    ds.writeStrLV(addrRange); 
    ds.writeTag(smeNTag);
    ds.writeInt32LV(smeN); 
    ds.writeTag(wantAliasTag);
    ds.writeBoolLV(wantAlias); 
    ds.writeTag(disabledTag);
    ds.writeBoolLV(disabled); 
    ds.writeTag(timeoutTag);
    ds.writeInt32LV(timeout); 
    ds.writeTag(receiptSchemeTag);
    ds.writeStrLV(receiptScheme); 
    ds.writeTag(modeTag);
    ds.writeByteLV(mode.getValue());
 
    ds.writeTag(procLimitTag);
    ds.writeInt32LV(procLimit); 
    ds.writeTag(schedLimitTag);
    ds.writeInt32LV(schedLimit); 
    ds.writeTag(accessMaskTag);
    ds.writeInt32LV(accessMask); 
    ds.writeTag(flagCarryOrgDescriptorTag);
    ds.writeBoolLV(flagCarryOrgDescriptor); 
    ds.writeTag(flagCarryOrgAbonentInfoTag);
    ds.writeBoolLV(flagCarryOrgAbonentInfo); 
    ds.writeTag(flagCarrySccpInfoTag);
    ds.writeBoolLV(flagCarrySccpInfo); 
    ds.writeTag(flagFillExtraDescriptorTag);
    ds.writeBoolLV(flagFillExtraDescriptor); 
    ds.writeTag(flagForceReceiptToSmeTag);
    ds.writeBoolLV(flagForceReceiptToSme); 
    ds.writeTag(flagForceGsmDatacodingTag);
    ds.writeBoolLV(flagForceGsmDatacoding); 
    ds.writeTag(flagSmppPlusTag);
    ds.writeBoolLV(flagSmppPlus); 
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
    //  throw protogen::framework::IncompatibleVersionException("SmeParams");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case idTag:
        {
          if(idFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("id");
          }
          id=ds.readStrLV();
          idFlag=true;
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
        case systemTypeTag:
        {
          if(systemTypeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("systemType");
          }
          systemType=ds.readStrLV();
          systemTypeFlag=true;
        }break;
        case passwordTag:
        {
          if(passwordFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("password");
          }
          password=ds.readStrLV();
          passwordFlag=true;
        }break;
        case addrRangeTag:
        {
          if(addrRangeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("addrRange");
          }
          addrRange=ds.readStrLV();
          addrRangeFlag=true;
        }break;
        case smeNTag:
        {
          if(smeNFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("smeN");
          }
          smeN=ds.readInt32LV();
          smeNFlag=true;
        }break;
        case wantAliasTag:
        {
          if(wantAliasFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("wantAlias");
          }
          wantAlias=ds.readBoolLV();
          wantAliasFlag=true;
        }break;
        case disabledTag:
        {
          if(disabledFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("disabled");
          }
          disabled=ds.readBoolLV();
          disabledFlag=true;
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
        case receiptSchemeTag:
        {
          if(receiptSchemeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("receiptScheme");
          }
          receiptScheme=ds.readStrLV();
          receiptSchemeFlag=true;
        }break;
        case modeTag:
        {
          if(modeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("mode");
          }
          mode=ds.readByteLV();
          modeFlag=true;
        }break;
        case procLimitTag:
        {
          if(procLimitFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("procLimit");
          }
          procLimit=ds.readInt32LV();
          procLimitFlag=true;
        }break;
        case schedLimitTag:
        {
          if(schedLimitFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("schedLimit");
          }
          schedLimit=ds.readInt32LV();
          schedLimitFlag=true;
        }break;
        case accessMaskTag:
        {
          if(accessMaskFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("accessMask");
          }
          accessMask=ds.readInt32LV();
          accessMaskFlag=true;
        }break;
        case flagCarryOrgDescriptorTag:
        {
          if(flagCarryOrgDescriptorFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("flagCarryOrgDescriptor");
          }
          flagCarryOrgDescriptor=ds.readBoolLV();
          flagCarryOrgDescriptorFlag=true;
        }break;
        case flagCarryOrgAbonentInfoTag:
        {
          if(flagCarryOrgAbonentInfoFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("flagCarryOrgAbonentInfo");
          }
          flagCarryOrgAbonentInfo=ds.readBoolLV();
          flagCarryOrgAbonentInfoFlag=true;
        }break;
        case flagCarrySccpInfoTag:
        {
          if(flagCarrySccpInfoFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("flagCarrySccpInfo");
          }
          flagCarrySccpInfo=ds.readBoolLV();
          flagCarrySccpInfoFlag=true;
        }break;
        case flagFillExtraDescriptorTag:
        {
          if(flagFillExtraDescriptorFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("flagFillExtraDescriptor");
          }
          flagFillExtraDescriptor=ds.readBoolLV();
          flagFillExtraDescriptorFlag=true;
        }break;
        case flagForceReceiptToSmeTag:
        {
          if(flagForceReceiptToSmeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("flagForceReceiptToSme");
          }
          flagForceReceiptToSme=ds.readBoolLV();
          flagForceReceiptToSmeFlag=true;
        }break;
        case flagForceGsmDatacodingTag:
        {
          if(flagForceGsmDatacodingFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("flagForceGsmDatacoding");
          }
          flagForceGsmDatacoding=ds.readBoolLV();
          flagForceGsmDatacodingFlag=true;
        }break;
        case flagSmppPlusTag:
        {
          if(flagSmppPlusFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("flagSmppPlus");
          }
          flagSmppPlus=ds.readBoolLV();
          flagSmppPlusFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("SmeParams",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!idFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!priorityFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("priority");
    }
    if(!systemTypeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("systemType");
    }
    if(!passwordFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("password");
    }
    if(!addrRangeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("addrRange");
    }
    if(!smeNFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("smeN");
    }
    if(!wantAliasFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("wantAlias");
    }
    if(!disabledFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("disabled");
    }
    if(!timeoutFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("timeout");
    }
    if(!receiptSchemeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("receiptScheme");
    }
    if(!modeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("mode");
    }
    if(!procLimitFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("procLimit");
    }
    if(!schedLimitFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("schedLimit");
    }
    if(!accessMaskFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("accessMask");
    }
    if(!flagCarryOrgDescriptorFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagCarryOrgDescriptor");
    }
    if(!flagCarryOrgAbonentInfoFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagCarryOrgAbonentInfo");
    }
    if(!flagCarrySccpInfoFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagCarrySccpInfo");
    }
    if(!flagFillExtraDescriptorFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagFillExtraDescriptor");
    }
    if(!flagForceReceiptToSmeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagForceReceiptToSme");
    }
    if(!flagForceGsmDatacodingFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagForceGsmDatacoding");
    }
    if(!flagSmppPlusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flagSmppPlus");
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t idTag=1;
  static const int32_t priorityTag=2;
  static const int32_t systemTypeTag=6;
  static const int32_t passwordTag=7;
  static const int32_t addrRangeTag=8;
  static const int32_t smeNTag=9;
  static const int32_t wantAliasTag=10;
  static const int32_t disabledTag=14;
  static const int32_t timeoutTag=12;
  static const int32_t receiptSchemeTag=13;
  static const int32_t modeTag=15;
  static const int32_t procLimitTag=16;
  static const int32_t schedLimitTag=17;
  static const int32_t accessMaskTag=18;
  static const int32_t flagCarryOrgDescriptorTag=19;
  static const int32_t flagCarryOrgAbonentInfoTag=20;
  static const int32_t flagCarrySccpInfoTag=21;
  static const int32_t flagFillExtraDescriptorTag=22;
  static const int32_t flagForceReceiptToSmeTag=23;
  static const int32_t flagForceGsmDatacodingTag=24;
  static const int32_t flagSmppPlusTag=25;


  std::string id;
  int32_t priority;
  std::string systemType;
  std::string password;
  std::string addrRange;
  int32_t smeN;
  bool wantAlias;
  bool disabled;
  int32_t timeout;
  std::string receiptScheme;
  SmeBindMode mode;
  int32_t procLimit;
  int32_t schedLimit;
  int32_t accessMask;
  bool flagCarryOrgDescriptor;
  bool flagCarryOrgAbonentInfo;
  bool flagCarrySccpInfo;
  bool flagFillExtraDescriptor;
  bool flagForceReceiptToSme;
  bool flagForceGsmDatacoding;
  bool flagSmppPlus;

  bool idFlag;
  bool priorityFlag;
  bool systemTypeFlag;
  bool passwordFlag;
  bool addrRangeFlag;
  bool smeNFlag;
  bool wantAliasFlag;
  bool disabledFlag;
  bool timeoutFlag;
  bool receiptSchemeFlag;
  bool modeFlag;
  bool procLimitFlag;
  bool schedLimitFlag;
  bool accessMaskFlag;
  bool flagCarryOrgDescriptorFlag;
  bool flagCarryOrgAbonentInfoFlag;
  bool flagCarrySccpInfoFlag;
  bool flagFillExtraDescriptorFlag;
  bool flagForceReceiptToSmeFlag;
  bool flagForceGsmDatacodingFlag;
  bool flagSmppPlusFlag;
};

}
}
}
}
#endif
