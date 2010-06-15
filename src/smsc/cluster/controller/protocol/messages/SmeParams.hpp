#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMEPARAMS_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMEPARAMS_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) SmeParams version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
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
    typeOfNumberFlag=false;
    numberingPlanFlag=false;
    interfaceVersionFlag=false;
    systemTypeFlag=false;
    passwordFlag=false;
    addrRangeFlag=false;
    smeNFlag=false;
    wantAliasFlag=false;
    timeoutFlag=false;
    receiptSchemeFlag=false;
    disabledFlag=false;
    modeFlag=false;
    procLimitFlag=false;
    schedLimitFlag=false;
    accessMaskFlag=false;
    flagsFlag=false;
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
    if(typeOfNumberFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="typeOfNumber=";
      sprintf(buf,"%d",typeOfNumber);
      rv+=buf;
    }
    if(numberingPlanFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="numberingPlan=";
      sprintf(buf,"%d",numberingPlan);
      rv+=buf;
    }
    if(interfaceVersionFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="interfaceVersion=";
      sprintf(buf,"%d",interfaceVersion);
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
    if(disabledFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="disabled=";
      rv+=disabled?"true":"false";
    }
    if(modeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="mode=";
      rv+=mode;
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
    if(flagsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flags=";
      sprintf(buf,"%d",flags);
      rv+=buf;
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
    if(typeOfNumberFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(typeOfNumber);
    }
    if(numberingPlanFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(numberingPlan);
    }
    if(interfaceVersionFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(interfaceVersion);
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
    if(disabledFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(disabled);
    }
    if(modeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(mode);
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
    if(flagsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(flags);
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
  int32_t getTypeOfNumber()const
  {
    if(!typeOfNumberFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("typeOfNumber");
    }
    return typeOfNumber;
  }
  void setTypeOfNumber(int32_t argValue)
  {
    typeOfNumber=argValue;
    typeOfNumberFlag=true;
  }
  int32_t& getTypeOfNumberRef()
  {
    typeOfNumberFlag=true;
    return typeOfNumber;
  }
  bool hasTypeOfNumber()const
  {
    return typeOfNumberFlag;
  }
  int32_t getNumberingPlan()const
  {
    if(!numberingPlanFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("numberingPlan");
    }
    return numberingPlan;
  }
  void setNumberingPlan(int32_t argValue)
  {
    numberingPlan=argValue;
    numberingPlanFlag=true;
  }
  int32_t& getNumberingPlanRef()
  {
    numberingPlanFlag=true;
    return numberingPlan;
  }
  bool hasNumberingPlan()const
  {
    return numberingPlanFlag;
  }
  int32_t getInterfaceVersion()const
  {
    if(!interfaceVersionFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("interfaceVersion");
    }
    return interfaceVersion;
  }
  void setInterfaceVersion(int32_t argValue)
  {
    interfaceVersion=argValue;
    interfaceVersionFlag=true;
  }
  int32_t& getInterfaceVersionRef()
  {
    interfaceVersionFlag=true;
    return interfaceVersion;
  }
  bool hasInterfaceVersion()const
  {
    return interfaceVersionFlag;
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
  const std::string& getMode()const
  {
    if(!modeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("mode");
    }
    return mode;
  }
  void setMode(const std::string& argValue)
  {
    mode=argValue;
    modeFlag=true;
  }
  std::string& getModeRef()
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
  int32_t getFlags()const
  {
    if(!flagsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("flags");
    }
    return flags;
  }
  void setFlags(int32_t argValue)
  {
    flags=argValue;
    flagsFlag=true;
  }
  int32_t& getFlagsRef()
  {
    flagsFlag=true;
    return flags;
  }
  bool hasFlags()const
  {
    return flagsFlag;
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
    if(!typeOfNumberFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("typeOfNumber");
    }
    if(!numberingPlanFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("numberingPlan");
    }
    if(!interfaceVersionFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("interfaceVersion");
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
    if(!timeoutFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("timeout");
    }
    if(!receiptSchemeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("receiptScheme");
    }
    if(!disabledFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("disabled");
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
    if(!flagsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flags");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(idTag);
    ds.writeStrLV(id);
    ds.writeTag(priorityTag);
    ds.writeInt32LV(priority);
    ds.writeTag(typeOfNumberTag);
    ds.writeInt32LV(typeOfNumber);
    ds.writeTag(numberingPlanTag);
    ds.writeInt32LV(numberingPlan);
    ds.writeTag(interfaceVersionTag);
    ds.writeInt32LV(interfaceVersion);
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
    ds.writeTag(timeoutTag);
    ds.writeInt32LV(timeout);
    ds.writeTag(receiptSchemeTag);
    ds.writeStrLV(receiptScheme);
    ds.writeTag(disabledTag);
    ds.writeBoolLV(disabled);
    ds.writeTag(modeTag);
    ds.writeStrLV(mode);
    ds.writeTag(procLimitTag);
    ds.writeInt32LV(procLimit);
    ds.writeTag(schedLimitTag);
    ds.writeInt32LV(schedLimit);
    ds.writeTag(accessMaskTag);
    ds.writeInt32LV(accessMask);
    ds.writeTag(flagsTag);
    ds.writeInt32LV(flags);
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
      DataStream::TagType tag=ds.readTag();
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
        case typeOfNumberTag:
        {
          if(typeOfNumberFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("typeOfNumber");
          }
          typeOfNumber=ds.readInt32LV();
          typeOfNumberFlag=true;
        }break;
        case numberingPlanTag:
        {
          if(numberingPlanFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("numberingPlan");
          }
          numberingPlan=ds.readInt32LV();
          numberingPlanFlag=true;
        }break;
        case interfaceVersionTag:
        {
          if(interfaceVersionFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("interfaceVersion");
          }
          interfaceVersion=ds.readInt32LV();
          interfaceVersionFlag=true;
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
        case disabledTag:
        {
          if(disabledFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("disabled");
          }
          disabled=ds.readBoolLV();
          disabledFlag=true;
        }break;
        case modeTag:
        {
          if(modeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("mode");
          }
          mode=ds.readStrLV();
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
        case flagsTag:
        {
          if(flagsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("flags");
          }
          flags=ds.readInt32LV();
          flagsFlag=true;
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
    if(!typeOfNumberFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("typeOfNumber");
    }
    if(!numberingPlanFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("numberingPlan");
    }
    if(!interfaceVersionFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("interfaceVersion");
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
    if(!timeoutFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("timeout");
    }
    if(!receiptSchemeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("receiptScheme");
    }
    if(!disabledFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("disabled");
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
    if(!flagsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flags");
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t idTag=1;
  static const int32_t priorityTag=2;
  static const int32_t typeOfNumberTag=3;
  static const int32_t numberingPlanTag=4;
  static const int32_t interfaceVersionTag=5;
  static const int32_t systemTypeTag=6;
  static const int32_t passwordTag=7;
  static const int32_t addrRangeTag=8;
  static const int32_t smeNTag=9;
  static const int32_t wantAliasTag=10;
  static const int32_t timeoutTag=12;
  static const int32_t receiptSchemeTag=13;
  static const int32_t disabledTag=14;
  static const int32_t modeTag=15;
  static const int32_t procLimitTag=16;
  static const int32_t schedLimitTag=17;
  static const int32_t accessMaskTag=18;
  static const int32_t flagsTag=19;


  std::string id;
  int32_t priority;
  int32_t typeOfNumber;
  int32_t numberingPlan;
  int32_t interfaceVersion;
  std::string systemType;
  std::string password;
  std::string addrRange;
  int32_t smeN;
  bool wantAlias;
  int32_t timeout;
  std::string receiptScheme;
  bool disabled;
  std::string mode;
  int32_t procLimit;
  int32_t schedLimit;
  int32_t accessMask;
  int32_t flags;

  bool idFlag;
  bool priorityFlag;
  bool typeOfNumberFlag;
  bool numberingPlanFlag;
  bool interfaceVersionFlag;
  bool systemTypeFlag;
  bool passwordFlag;
  bool addrRangeFlag;
  bool smeNFlag;
  bool wantAliasFlag;
  bool timeoutFlag;
  bool receiptSchemeFlag;
  bool disabledFlag;
  bool modeFlag;
  bool procLimitFlag;
  bool schedLimitFlag;
  bool accessMaskFlag;
  bool flagsFlag;
};

}
}
}
}
}
#endif
