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

typedef std::vector<std::string> string_list;

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
      sprintf(buf,"%u",(unsigned int)priority);
      rv+=buf;
    }
    if(typeOfNumberFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="typeOfNumber=";
      sprintf(buf,"%u",(unsigned int)typeOfNumber);
      rv+=buf;
    }
    if(numberingPlanFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="numberingPlan=";
      sprintf(buf,"%u",(unsigned int)numberingPlan);
      rv+=buf;
    }
    if(interfaceVersionFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="interfaceVersion=";
      sprintf(buf,"%u",(unsigned int)interfaceVersion);
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
      sprintf(buf,"%u",(unsigned int)smeN);
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
      sprintf(buf,"%u",(unsigned int)timeout);
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
      sprintf(buf,"%u",(unsigned int)procLimit);
      rv+=buf;
    }
    if(schedLimitFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="schedLimit=";
      sprintf(buf,"%u",(unsigned int)schedLimit);
      rv+=buf;
    }
    if(accessMaskFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="accessMask=";
      sprintf(buf,"%u",(unsigned int)accessMask);
      rv+=buf;
    }
    if(flagsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flags=";
      sprintf(buf,"%u",(unsigned int)flags);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
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
      throw protogen::framework::FieldIsNullException("id");
    }
    return id;
  }
  void setId(const std::string& value)
  {
    id=value;
    idFlag=true;
  }
  bool hasId()const
  {
    return idFlag;
  }
  uint32_t getPriority()const
  {
    if(!priorityFlag)
    {
      throw protogen::framework::FieldIsNullException("priority");
    }
    return priority;
  }
  void setPriority(uint32_t value)
  {
    priority=value;
    priorityFlag=true;
  }
  bool hasPriority()const
  {
    return priorityFlag;
  }
  uint32_t getTypeOfNumber()const
  {
    if(!typeOfNumberFlag)
    {
      throw protogen::framework::FieldIsNullException("typeOfNumber");
    }
    return typeOfNumber;
  }
  void setTypeOfNumber(uint32_t value)
  {
    typeOfNumber=value;
    typeOfNumberFlag=true;
  }
  bool hasTypeOfNumber()const
  {
    return typeOfNumberFlag;
  }
  uint32_t getNumberingPlan()const
  {
    if(!numberingPlanFlag)
    {
      throw protogen::framework::FieldIsNullException("numberingPlan");
    }
    return numberingPlan;
  }
  void setNumberingPlan(uint32_t value)
  {
    numberingPlan=value;
    numberingPlanFlag=true;
  }
  bool hasNumberingPlan()const
  {
    return numberingPlanFlag;
  }
  uint32_t getInterfaceVersion()const
  {
    if(!interfaceVersionFlag)
    {
      throw protogen::framework::FieldIsNullException("interfaceVersion");
    }
    return interfaceVersion;
  }
  void setInterfaceVersion(uint32_t value)
  {
    interfaceVersion=value;
    interfaceVersionFlag=true;
  }
  bool hasInterfaceVersion()const
  {
    return interfaceVersionFlag;
  }
  const std::string& getSystemType()const
  {
    if(!systemTypeFlag)
    {
      throw protogen::framework::FieldIsNullException("systemType");
    }
    return systemType;
  }
  void setSystemType(const std::string& value)
  {
    systemType=value;
    systemTypeFlag=true;
  }
  bool hasSystemType()const
  {
    return systemTypeFlag;
  }
  const std::string& getPassword()const
  {
    if(!passwordFlag)
    {
      throw protogen::framework::FieldIsNullException("password");
    }
    return password;
  }
  void setPassword(const std::string& value)
  {
    password=value;
    passwordFlag=true;
  }
  bool hasPassword()const
  {
    return passwordFlag;
  }
  const std::string& getAddrRange()const
  {
    if(!addrRangeFlag)
    {
      throw protogen::framework::FieldIsNullException("addrRange");
    }
    return addrRange;
  }
  void setAddrRange(const std::string& value)
  {
    addrRange=value;
    addrRangeFlag=true;
  }
  bool hasAddrRange()const
  {
    return addrRangeFlag;
  }
  uint32_t getSmeN()const
  {
    if(!smeNFlag)
    {
      throw protogen::framework::FieldIsNullException("smeN");
    }
    return smeN;
  }
  void setSmeN(uint32_t value)
  {
    smeN=value;
    smeNFlag=true;
  }
  bool hasSmeN()const
  {
    return smeNFlag;
  }
  bool getWantAlias()const
  {
    if(!wantAliasFlag)
    {
      throw protogen::framework::FieldIsNullException("wantAlias");
    }
    return wantAlias;
  }
  void setWantAlias(bool value)
  {
    wantAlias=value;
    wantAliasFlag=true;
  }
  bool hasWantAlias()const
  {
    return wantAliasFlag;
  }
  uint32_t getTimeout()const
  {
    if(!timeoutFlag)
    {
      throw protogen::framework::FieldIsNullException("timeout");
    }
    return timeout;
  }
  void setTimeout(uint32_t value)
  {
    timeout=value;
    timeoutFlag=true;
  }
  bool hasTimeout()const
  {
    return timeoutFlag;
  }
  const std::string& getReceiptScheme()const
  {
    if(!receiptSchemeFlag)
    {
      throw protogen::framework::FieldIsNullException("receiptScheme");
    }
    return receiptScheme;
  }
  void setReceiptScheme(const std::string& value)
  {
    receiptScheme=value;
    receiptSchemeFlag=true;
  }
  bool hasReceiptScheme()const
  {
    return receiptSchemeFlag;
  }
  bool getDisabled()const
  {
    if(!disabledFlag)
    {
      throw protogen::framework::FieldIsNullException("disabled");
    }
    return disabled;
  }
  void setDisabled(bool value)
  {
    disabled=value;
    disabledFlag=true;
  }
  bool hasDisabled()const
  {
    return disabledFlag;
  }
  const std::string& getMode()const
  {
    if(!modeFlag)
    {
      throw protogen::framework::FieldIsNullException("mode");
    }
    return mode;
  }
  void setMode(const std::string& value)
  {
    mode=value;
    modeFlag=true;
  }
  bool hasMode()const
  {
    return modeFlag;
  }
  uint32_t getProcLimit()const
  {
    if(!procLimitFlag)
    {
      throw protogen::framework::FieldIsNullException("procLimit");
    }
    return procLimit;
  }
  void setProcLimit(uint32_t value)
  {
    procLimit=value;
    procLimitFlag=true;
  }
  bool hasProcLimit()const
  {
    return procLimitFlag;
  }
  uint32_t getSchedLimit()const
  {
    if(!schedLimitFlag)
    {
      throw protogen::framework::FieldIsNullException("schedLimit");
    }
    return schedLimit;
  }
  void setSchedLimit(uint32_t value)
  {
    schedLimit=value;
    schedLimitFlag=true;
  }
  bool hasSchedLimit()const
  {
    return schedLimitFlag;
  }
  uint32_t getAccessMask()const
  {
    if(!accessMaskFlag)
    {
      throw protogen::framework::FieldIsNullException("accessMask");
    }
    return accessMask;
  }
  void setAccessMask(uint32_t value)
  {
    accessMask=value;
    accessMaskFlag=true;
  }
  bool hasAccessMask()const
  {
    return accessMaskFlag;
  }
  uint32_t getFlags()const
  {
    if(!flagsFlag)
    {
      throw protogen::framework::FieldIsNullException("flags");
    }
    return flags;
  }
  void setFlags(uint32_t value)
  {
    flags=value;
    flagsFlag=true;
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
      throw protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!priorityFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("priority");
    }
    if(!typeOfNumberFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("typeOfNumber");
    }
    if(!numberingPlanFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("numberingPlan");
    }
    if(!interfaceVersionFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("interfaceVersion");
    }
    if(!systemTypeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("systemType");
    }
    if(!passwordFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("password");
    }
    if(!addrRangeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("addrRange");
    }
    if(!smeNFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("smeN");
    }
    if(!wantAliasFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("wantAlias");
    }
    if(!timeoutFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("timeout");
    }
    if(!receiptSchemeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("receiptScheme");
    }
    if(!disabledFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("disabled");
    }
    if(!modeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("mode");
    }
    if(!procLimitFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("procLimit");
    }
    if(!schedLimitFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("schedLimit");
    }
    if(!accessMaskFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("accessMask");
    }
    if(!flagsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("flags");
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
    //uint8_t rdVersionMajor=ds.readByte();
    //uint8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("SmeParams");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case idTag:
        {
          if(idFlag)
          {
            throw protogen::framework::DuplicateFieldException("id");
          }
          id=ds.readStrLV();
          idFlag=true;
        }break;
        case priorityTag:
        {
          if(priorityFlag)
          {
            throw protogen::framework::DuplicateFieldException("priority");
          }
          priority=ds.readInt32LV();
          priorityFlag=true;
        }break;
        case typeOfNumberTag:
        {
          if(typeOfNumberFlag)
          {
            throw protogen::framework::DuplicateFieldException("typeOfNumber");
          }
          typeOfNumber=ds.readInt32LV();
          typeOfNumberFlag=true;
        }break;
        case numberingPlanTag:
        {
          if(numberingPlanFlag)
          {
            throw protogen::framework::DuplicateFieldException("numberingPlan");
          }
          numberingPlan=ds.readInt32LV();
          numberingPlanFlag=true;
        }break;
        case interfaceVersionTag:
        {
          if(interfaceVersionFlag)
          {
            throw protogen::framework::DuplicateFieldException("interfaceVersion");
          }
          interfaceVersion=ds.readInt32LV();
          interfaceVersionFlag=true;
        }break;
        case systemTypeTag:
        {
          if(systemTypeFlag)
          {
            throw protogen::framework::DuplicateFieldException("systemType");
          }
          systemType=ds.readStrLV();
          systemTypeFlag=true;
        }break;
        case passwordTag:
        {
          if(passwordFlag)
          {
            throw protogen::framework::DuplicateFieldException("password");
          }
          password=ds.readStrLV();
          passwordFlag=true;
        }break;
        case addrRangeTag:
        {
          if(addrRangeFlag)
          {
            throw protogen::framework::DuplicateFieldException("addrRange");
          }
          addrRange=ds.readStrLV();
          addrRangeFlag=true;
        }break;
        case smeNTag:
        {
          if(smeNFlag)
          {
            throw protogen::framework::DuplicateFieldException("smeN");
          }
          smeN=ds.readInt32LV();
          smeNFlag=true;
        }break;
        case wantAliasTag:
        {
          if(wantAliasFlag)
          {
            throw protogen::framework::DuplicateFieldException("wantAlias");
          }
          wantAlias=ds.readBoolLV();
          wantAliasFlag=true;
        }break;
        case timeoutTag:
        {
          if(timeoutFlag)
          {
            throw protogen::framework::DuplicateFieldException("timeout");
          }
          timeout=ds.readInt32LV();
          timeoutFlag=true;
        }break;
        case receiptSchemeTag:
        {
          if(receiptSchemeFlag)
          {
            throw protogen::framework::DuplicateFieldException("receiptScheme");
          }
          receiptScheme=ds.readStrLV();
          receiptSchemeFlag=true;
        }break;
        case disabledTag:
        {
          if(disabledFlag)
          {
            throw protogen::framework::DuplicateFieldException("disabled");
          }
          disabled=ds.readBoolLV();
          disabledFlag=true;
        }break;
        case modeTag:
        {
          if(modeFlag)
          {
            throw protogen::framework::DuplicateFieldException("mode");
          }
          mode=ds.readStrLV();
          modeFlag=true;
        }break;
        case procLimitTag:
        {
          if(procLimitFlag)
          {
            throw protogen::framework::DuplicateFieldException("procLimit");
          }
          procLimit=ds.readInt32LV();
          procLimitFlag=true;
        }break;
        case schedLimitTag:
        {
          if(schedLimitFlag)
          {
            throw protogen::framework::DuplicateFieldException("schedLimit");
          }
          schedLimit=ds.readInt32LV();
          schedLimitFlag=true;
        }break;
        case accessMaskTag:
        {
          if(accessMaskFlag)
          {
            throw protogen::framework::DuplicateFieldException("accessMask");
          }
          accessMask=ds.readInt32LV();
          accessMaskFlag=true;
        }break;
        case flagsTag:
        {
          if(flagsFlag)
          {
            throw protogen::framework::DuplicateFieldException("flags");
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
      throw protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!priorityFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("priority");
    }
    if(!typeOfNumberFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("typeOfNumber");
    }
    if(!numberingPlanFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("numberingPlan");
    }
    if(!interfaceVersionFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("interfaceVersion");
    }
    if(!systemTypeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("systemType");
    }
    if(!passwordFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("password");
    }
    if(!addrRangeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("addrRange");
    }
    if(!smeNFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("smeN");
    }
    if(!wantAliasFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("wantAlias");
    }
    if(!timeoutFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("timeout");
    }
    if(!receiptSchemeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("receiptScheme");
    }
    if(!disabledFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("disabled");
    }
    if(!modeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("mode");
    }
    if(!procLimitFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("procLimit");
    }
    if(!schedLimitFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("schedLimit");
    }
    if(!accessMaskFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("accessMask");
    }
    if(!flagsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("flags");
    }

  }


protected:
  //static const uint8_t versionMajor=1;
  //static const uint8_t versionMinor=0;

  static const uint32_t idTag=1;
  static const uint32_t priorityTag=2;
  static const uint32_t typeOfNumberTag=3;
  static const uint32_t numberingPlanTag=4;
  static const uint32_t interfaceVersionTag=5;
  static const uint32_t systemTypeTag=6;
  static const uint32_t passwordTag=7;
  static const uint32_t addrRangeTag=8;
  static const uint32_t smeNTag=9;
  static const uint32_t wantAliasTag=10;
  static const uint32_t timeoutTag=12;
  static const uint32_t receiptSchemeTag=13;
  static const uint32_t disabledTag=14;
  static const uint32_t modeTag=15;
  static const uint32_t procLimitTag=16;
  static const uint32_t schedLimitTag=17;
  static const uint32_t accessMaskTag=18;
  static const uint32_t flagsTag=19;


  std::string id;
  uint32_t priority;
  uint32_t typeOfNumber;
  uint32_t numberingPlan;
  uint32_t interfaceVersion;
  std::string systemType;
  std::string password;
  std::string addrRange;
  uint32_t smeN;
  bool wantAlias;
  uint32_t timeout;
  std::string receiptScheme;
  bool disabled;
  std::string mode;
  uint32_t procLimit;
  uint32_t schedLimit;
  uint32_t accessMask;
  uint32_t flags;

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
