// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_BC_INC_MOD_HPP__
#define __SCAG_PVSS_PVAP_BC_INC_MOD_HPP__

#include "util/int.h"
#include <string>
#include "Exceptions.h"


namespace scag{
namespace pvss{
namespace pvap{

class BC_INC_MOD : public BC_CMD 
{
public:
    BC_INC_MOD()
    {
        clear();
    }
    void clear()
    {
        varNameFlag=false;
        valueTypeFlag=false;
        timePolicyFlag=false;
        finalDateFlag=false;
        lifeTimeFlag=false;
        intValueFlag=false;
        modValueFlag=false;
    }

    std::string toString() const
    {
        std::string rv("BC_INC_MOD:");
        char buf[32];
        sprintf(buf,"seqNum=%d",seqNum);
        rv+=buf;
        if(varNameFlag) {
            rv+=";varName=";
            rv+=varName;
        }
        if(valueTypeFlag) {
            rv+=";valueType=";
            sprintf(buf,"%u",(unsigned int)valueType);
            rv+=buf;
        }
        if(timePolicyFlag) {
            rv+=";timePolicy=";
            sprintf(buf,"%u",(unsigned int)timePolicy);
            rv+=buf;
        }
        if(finalDateFlag) {
            rv+=";finalDate=";
            sprintf(buf,"%u",(unsigned int)finalDate);
            rv+=buf;
        }
        if(lifeTimeFlag) {
            rv+=";lifeTime=";
            sprintf(buf,"%u",(unsigned int)lifeTime);
            rv+=buf;
        }
        if(intValueFlag) {
            rv+=";intValue=";
            sprintf(buf,"%u",(unsigned int)intValue);
            rv+=buf;
        }
        if(modValueFlag) {
            rv+=";modValue=";
            sprintf(buf,"%u",(unsigned int)modValue);
            rv+=buf;
        }
        return rv;
    }

    template <class DataStream> uint32_t length()const
    {
        uint32_t rv=0;
        if (varNameFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(varName);
        }
        if (valueTypeFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(valueType);
        }
        if (timePolicyFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(timePolicy);
        }
        if (finalDateFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(finalDate);
        }
        if (lifeTimeFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(lifeTime);
        }
        if (intValueFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(intValue);
        }
        if (modValueFlag) {
            rv+=DataStream::tagTypeSize;
            rv+=DataStream::lengthTypeSize;
            rv+=DataStream::fieldSize(modValue);
        }
        return rv;
    }

  const std::string& getVarName() const
    {
        if (!varNameFlag) {
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
  uint8_t getValueType() const
    {
        if (!valueTypeFlag) {
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
  uint8_t getTimePolicy() const
    {
        if (!timePolicyFlag) {
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
  uint32_t getFinalDate() const
    {
        if (!finalDateFlag) {
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
  uint32_t getLifeTime() const
    {
        if (!lifeTimeFlag) {
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
  uint32_t getIntValue() const
    {
        if (!intValueFlag) {
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
  uint32_t getModValue() const
    {
        if (!modValueFlag) {
            throw FieldIsNullException("modValue");
        }
        return modValue;
    }

    void setModValue(uint32_t value)
    {
        modValue=value;
        modValueFlag=true;
    }
    bool hasModValue()const
    {
        return modValueFlag;
    }

    template <class DataStream> void serialize(DataStream& ds) const
    {
        checkFields();
        // mandatory fields
        ds.writeTag(varNameTag);
    ds.writeStrLV(varName);
        ds.writeTag(valueTypeTag);
    ds.writeByteLV(valueType);
        ds.writeTag(timePolicyTag);
    ds.writeByteLV(timePolicy);
        ds.writeTag(finalDateTag);
    ds.writeInt32LV(finalDate);
        ds.writeTag(lifeTimeTag);
    ds.writeInt32LV(lifeTime);
        ds.writeTag(intValueTag);
    ds.writeInt32LV(intValue);
        ds.writeTag(modValueTag);
    ds.writeInt32LV(modValue);
        // optional fields
        //ds.writeTag(DataStream::endOfMessage_tag);
    }

    template <class DataStream> void deserialize(DataStream& ds)
    {
        clear();
        bool endOfMessage=false;
        //uint8_t rdVersionMajor=ds.readByte();
        //uint8_t rdVersionMinor=ds.readByte();
        //if(rdVersionMajor!=versionMajor)
        //{
        //  throw IncompatibleVersionException("BC_INC_MOD");
        //}
        //seqNum=ds.readInt32();
        while (!endOfMessage) {
            uint32_t tag=ds.readTag();
            switch(tag) {
            case varNameTag: {
                if (varNameFlag) {
                    throw DuplicateFieldException("varName");
                }
          varName=ds.readStrLV();
                varNameFlag=true;
                break;
            }
            case valueTypeTag: {
                if (valueTypeFlag) {
                    throw DuplicateFieldException("valueType");
                }
          valueType=ds.readByteLV();
                valueTypeFlag=true;
                break;
            }
            case timePolicyTag: {
                if (timePolicyFlag) {
                    throw DuplicateFieldException("timePolicy");
                }
          timePolicy=ds.readByteLV();
                timePolicyFlag=true;
                break;
            }
            case finalDateTag: {
                if (finalDateFlag) {
                    throw DuplicateFieldException("finalDate");
                }
          finalDate=ds.readInt32LV();
                finalDateFlag=true;
                break;
            }
            case lifeTimeTag: {
                if (lifeTimeFlag) {
                    throw DuplicateFieldException("lifeTime");
                }
          lifeTime=ds.readInt32LV();
                lifeTimeFlag=true;
                break;
            }
            case intValueTag: {
                if (intValueFlag) {
                    throw DuplicateFieldException("intValue");
                }
          intValue=ds.readInt32LV();
                intValueFlag=true;
                break;
            }
            case modValueTag: {
                if (modValueFlag) {
                    throw DuplicateFieldException("modValue");
                }
          modValue=ds.readInt32LV();
                modValueFlag=true;
                break;
            }
            case DataStream::endOfMessage_tag:
                endOfMessage=true;
                break;
            default:
                //if(rdVersionMinor==versionMinor)
                //{
                //  throw UnexpectedTag("BC_INC_MOD",tag);
                //}
                ds.skip(ds.readLength());
            }
        }
        checkFields();
    }

    uint32_t getSeqNum() const
    {
        return seqNum;
    }
 
    void setSeqNum(uint32_t value)
    {
        seqNum=value;
    }

protected:
    void checkFields() const throw (MandatoryFieldMissingException)
    {
        // checking mandatory fields
        if (!varNameFlag) {
            throw MandatoryFieldMissingException("varName");
        }
        if (!valueTypeFlag) {
            throw MandatoryFieldMissingException("valueType");
        }
        if (!timePolicyFlag) {
            throw MandatoryFieldMissingException("timePolicy");
        }
        if (!finalDateFlag) {
            throw MandatoryFieldMissingException("finalDate");
        }
        if (!lifeTimeFlag) {
            throw MandatoryFieldMissingException("lifeTime");
        }
        if (!intValueFlag) {
            throw MandatoryFieldMissingException("intValue");
        }
        if (!modValueFlag) {
            throw MandatoryFieldMissingException("modValue");
        }
        // checking optional fields
    }

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;

    static const uint16_t varNameTag=5;
    static const uint16_t valueTypeTag=6;
    static const uint16_t timePolicyTag=7;
    static const uint16_t finalDateTag=8;
    static const uint16_t lifeTimeTag=9;
    static const uint16_t intValueTag=10;
    static const uint16_t modValueTag=14;

    uint32_t seqNum;

    std::string varName;
    uint8_t valueType;
    uint8_t timePolicy;
    uint32_t finalDate;
    uint32_t lifeTime;
    uint32_t intValue;
    uint32_t modValue;

    bool varNameFlag;
    bool valueTypeFlag;
    bool timePolicyFlag;
    bool finalDateFlag;
    bool lifeTimeFlag;
    bool intValueFlag;
    bool modValueFlag;
};

} // namespace scag
} // namespace pvss
} // namespace pvap
#endif
