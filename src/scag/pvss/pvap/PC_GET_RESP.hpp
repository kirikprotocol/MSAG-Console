// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_PC_GET_RESP_HPP__
#define __SCAG_PVSS_PVAP_PC_GET_RESP_HPP__

#include "util/int.h"
#include <string>
#include "Exceptions.h"


namespace scag{
namespace pvss{
namespace pvap{

class PC_GET_RESP  {
public:
  PC_GET_RESP()
  {
    Clear();
  }
  void Clear()
  {
    statusFlag=false;
    valueTypeFlag=false;
    timePolicyFlag=false;
    finalDateFlag=false;
    lifeTimeFlag=false;
    varNameFlag=false;
    intValueFlag=false;
    stringValueFlag=false;
    boolValueFlag=false;
    dateValueFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
		sprintf(buf,"seqNum=%d",seqNum);
		rv+=buf;
    if(statusFlag)
    {
      rv+=";status=";
      sprintf(buf,"%u",(unsigned int)status);
      rv+=buf;
    }
    if(valueTypeFlag)
    {
      rv+=";valueType=";
      sprintf(buf,"%u",(unsigned int)valueType);
      rv+=buf;
    }
    if(timePolicyFlag)
    {
      rv+=";timePolicy=";
      sprintf(buf,"%u",(unsigned int)timePolicy);
      rv+=buf;
    }
    if(finalDateFlag)
    {
      rv+=";finalDate=";
      sprintf(buf,"%u",(unsigned int)finalDate);
      rv+=buf;
    }
    if(lifeTimeFlag)
    {
      rv+=";lifeTime=";
      sprintf(buf,"%u",(unsigned int)lifeTime);
      rv+=buf;
    }
    if(varNameFlag)
    {
      rv+=";varName=";
      rv+=varName;
    }
    if(intValueFlag)
    {
      rv+=";intValue=";
      sprintf(buf,"%u",(unsigned int)intValue);
      rv+=buf;
    }
    if(stringValueFlag)
    {
      rv+=";stringValue=";
      rv+=stringValue;
    }
    if(boolValueFlag)
    {
      rv+=";boolValue=";
      sprintf(buf,"%u",(unsigned int)boolValue);
      rv+=buf;
    }
    if(dateValueFlag)
    {
      rv+=";dateValue=";
      sprintf(buf,"%u",(unsigned int)dateValue);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(statusFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(status);
    }
    if(valueTypeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(valueType);
    }
    if(timePolicyFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(timePolicy);
    }
    if(finalDateFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(finalDate);
    }
    if(lifeTimeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(lifeTime);
    }
    if(varNameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(varName);
    }
    if(intValueFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(intValue);
    }
    if(stringValueFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(stringValue);
    }
    if(boolValueFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(boolValue);
    }
    if(dateValueFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(dateValue);
    }

    return rv;
  }
  uint8_t getStatus()const
  {
    if(!statusFlag)
    {
      throw FieldIsNullException("status");
    }
    return status;
  }
  void setStatus(uint8_t value)
  {
    status=value;
    statusFlag=true;
  }
  bool hasStatus()const
  {
    return statusFlag;
  }
  uint8_t getValueType()const
  {
    if(!valueTypeFlag)
    {
      throw FieldIsNullException("valueType");
    }
    return valueType;
  }
  void setValueType(uint8_t value)
  {
    valueType=value;
    valueTypeFlag=true;
  }
  bool hasValueType()const
  {
    return valueTypeFlag;
  }
  uint8_t getTimePolicy()const
  {
    if(!timePolicyFlag)
    {
      throw FieldIsNullException("timePolicy");
    }
    return timePolicy;
  }
  void setTimePolicy(uint8_t value)
  {
    timePolicy=value;
    timePolicyFlag=true;
  }
  bool hasTimePolicy()const
  {
    return timePolicyFlag;
  }
  uint32_t getFinalDate()const
  {
    if(!finalDateFlag)
    {
      throw FieldIsNullException("finalDate");
    }
    return finalDate;
  }
  void setFinalDate(uint32_t value)
  {
    finalDate=value;
    finalDateFlag=true;
  }
  bool hasFinalDate()const
  {
    return finalDateFlag;
  }
  uint32_t getLifeTime()const
  {
    if(!lifeTimeFlag)
    {
      throw FieldIsNullException("lifeTime");
    }
    return lifeTime;
  }
  void setLifeTime(uint32_t value)
  {
    lifeTime=value;
    lifeTimeFlag=true;
  }
  bool hasLifeTime()const
  {
    return lifeTimeFlag;
  }
  const std::string& getVarName()const
  {
    if(!varNameFlag)
    {
      throw FieldIsNullException("varName");
    }
    return varName;
  }
  void setVarName(const std::string& value)
  {
    varName=value;
    varNameFlag=true;
  }
  bool hasVarName()const
  {
    return varNameFlag;
  }
  uint32_t getIntValue()const
  {
    if(!intValueFlag)
    {
      throw FieldIsNullException("intValue");
    }
    return intValue;
  }
  void setIntValue(uint32_t value)
  {
    intValue=value;
    intValueFlag=true;
  }
  bool hasIntValue()const
  {
    return intValueFlag;
  }
  const std::string& getStringValue()const
  {
    if(!stringValueFlag)
    {
      throw FieldIsNullException("stringValue");
    }
    return stringValue;
  }
  void setStringValue(const std::string& value)
  {
    stringValue=value;
    stringValueFlag=true;
  }
  bool hasStringValue()const
  {
    return stringValueFlag;
  }
  uint8_t getBoolValue()const
  {
    if(!boolValueFlag)
    {
      throw FieldIsNullException("boolValue");
    }
    return boolValue;
  }
  void setBoolValue(uint8_t value)
  {
    boolValue=value;
    boolValueFlag=true;
  }
  bool hasBoolValue()const
  {
    return boolValueFlag;
  }
  uint32_t getDateValue()const
  {
    if(!dateValueFlag)
    {
      throw FieldIsNullException("dateValue");
    }
    return dateValue;
  }
  void setDateValue(uint32_t value)
  {
    dateValue=value;
    dateValueFlag=true;
  }
  bool hasDateValue()const
  {
    return dateValueFlag;
  }

  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!statusFlag)
    {
      throw MandatoryFieldMissingException("status");
    }
    if(!valueTypeFlag)
    {
      throw MandatoryFieldMissingException("valueType");
    }
    if(!timePolicyFlag)
    {
      throw MandatoryFieldMissingException("timePolicy");
    }
    if(!finalDateFlag)
    {
      throw MandatoryFieldMissingException("finalDate");
    }
    if(!lifeTimeFlag)
    {
      throw MandatoryFieldMissingException("lifeTime");
    }
    if(!varNameFlag)
    {
      throw MandatoryFieldMissingException("varName");
    }
    // checking profile type
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(statusTag);
    ds.writeByteLV(status);
    ds.writeTag(valueTypeTag);
    ds.writeByteLV(valueType);
    ds.writeTag(timePolicyTag);
    ds.writeByteLV(timePolicy);
    ds.writeTag(finalDateTag);
    ds.writeInt32LV(finalDate);
    ds.writeTag(lifeTimeTag);
    ds.writeInt32LV(lifeTime);
    ds.writeTag(varNameTag);
    ds.writeStrLV(varName);
    if(intValueFlag)
    {
      ds.writeTag(intValueTag);
      ds.writeInt32LV(intValue);
    }
    if(stringValueFlag)
    {
      ds.writeTag(stringValueTag);
      ds.writeStrLV(stringValue);
    }
    if(boolValueFlag)
    {
      ds.writeTag(boolValueTag);
      ds.writeByteLV(boolValue);
    }
    if(dateValueFlag)
    {
      ds.writeTag(dateValueTag);
      ds.writeInt32LV(dateValue);
    }
    //ds.writeTag(DataStream::endOfMessage_tag);
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
    //  throw IncompatibleVersionException("PC_GET_RESP");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case statusTag:
        {
          if(statusFlag)
          {
            throw DuplicateFieldException("status");
          }
          status=ds.readByteLV();
          statusFlag=true;
        }break;
        case valueTypeTag:
        {
          if(valueTypeFlag)
          {
            throw DuplicateFieldException("valueType");
          }
          valueType=ds.readByteLV();
          valueTypeFlag=true;
        }break;
        case timePolicyTag:
        {
          if(timePolicyFlag)
          {
            throw DuplicateFieldException("timePolicy");
          }
          timePolicy=ds.readByteLV();
          timePolicyFlag=true;
        }break;
        case finalDateTag:
        {
          if(finalDateFlag)
          {
            throw DuplicateFieldException("finalDate");
          }
          finalDate=ds.readInt32LV();
          finalDateFlag=true;
        }break;
        case lifeTimeTag:
        {
          if(lifeTimeFlag)
          {
            throw DuplicateFieldException("lifeTime");
          }
          lifeTime=ds.readInt32LV();
          lifeTimeFlag=true;
        }break;
        case varNameTag:
        {
          if(varNameFlag)
          {
            throw DuplicateFieldException("varName");
          }
          varName=ds.readStrLV();
          varNameFlag=true;
        }break;
        case intValueTag:
        {
          if(intValueFlag)
          {
            throw DuplicateFieldException("intValue");
          }
          intValue=ds.readInt32LV();
          intValueFlag=true;
        }break;
        case stringValueTag:
        {
          if(stringValueFlag)
          {
            throw DuplicateFieldException("stringValue");
          }
          stringValue=ds.readStrLV();
          stringValueFlag=true;
        }break;
        case boolValueTag:
        {
          if(boolValueFlag)
          {
            throw DuplicateFieldException("boolValue");
          }
          boolValue=ds.readByteLV();
          boolValueFlag=true;
        }break;
        case dateValueTag:
        {
          if(dateValueFlag)
          {
            throw DuplicateFieldException("dateValue");
          }
          dateValue=ds.readInt32LV();
          dateValueFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw UnexpectedTag("PC_GET_RESP",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!statusFlag)
    {
      throw MandatoryFieldMissingException("status");
    }
    if(!valueTypeFlag)
    {
      throw MandatoryFieldMissingException("valueType");
    }
    if(!timePolicyFlag)
    {
      throw MandatoryFieldMissingException("timePolicy");
    }
    if(!finalDateFlag)
    {
      throw MandatoryFieldMissingException("finalDate");
    }
    if(!lifeTimeFlag)
    {
      throw MandatoryFieldMissingException("lifeTime");
    }
    if(!varNameFlag)
    {
      throw MandatoryFieldMissingException("varName");
    }
    // checking profile type
  }

  uint32_t getSeqNum()const
  {
    return seqNum;
  }
 
  void setSeqNum(uint32_t value)
  {
    seqNum=value;
  }

protected:
  //static const uint8_t versionMajor=2;
  //static const uint8_t versionMinor=0;

  static const uint32_t statusTag=1;
  static const uint32_t valueTypeTag=6;
  static const uint32_t timePolicyTag=7;
  static const uint32_t finalDateTag=8;
  static const uint32_t lifeTimeTag=9;
  static const uint32_t varNameTag=5;
  static const uint32_t intValueTag=10;
  static const uint32_t stringValueTag=11;
  static const uint32_t boolValueTag=12;
  static const uint32_t dateValueTag=13;

  uint32_t seqNum;

  uint8_t status;
  uint8_t valueType;
  uint8_t timePolicy;
  uint32_t finalDate;
  uint32_t lifeTime;
  std::string varName;
  uint32_t intValue;
  std::string stringValue;
  uint8_t boolValue;
  uint32_t dateValue;

  bool statusFlag;
  bool valueTypeFlag;
  bool timePolicyFlag;
  bool finalDateFlag;
  bool lifeTimeFlag;
  bool varNameFlag;
  bool intValueFlag;
  bool stringValueFlag;
  bool boolValueFlag;
  bool dateValueFlag;
};

}
}
}
#endif
