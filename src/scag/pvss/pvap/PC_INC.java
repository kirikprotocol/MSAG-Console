package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_INC  
{
    // static Logger logger = Logger.getLogger(PC_INC.class);

    static final int profileTypeTag = 2;
    static final int abonentKeyTag = 3;
    static final int profileKeyTag = 4;
    static final int varNameTag = 5;
    static final int valueTypeTag = 6;
    static final int timePolicyTag = 7;
    static final int finalDateTag = 8;
    static final int lifeTimeTag = 9;
    static final int intValueTag = 10;

    int seqNum;
    byte profileType;
    boolean profileTypeFlag=false;
    String abonentKey;
    boolean abonentKeyFlag=false;
    int profileKey;
    boolean profileKeyFlag=false;
    String varName;
    boolean varNameFlag=false;
    byte valueType;
    boolean valueTypeFlag=false;
    byte timePolicy;
    boolean timePolicyFlag=false;
    int finalDate;
    boolean finalDateFlag=false;
    int lifeTime;
    boolean lifeTimeFlag=false;
    int intValue;
    boolean intValueFlag=false;

    public PC_INC() {
    }

    public PC_INC(int seqNum, byte profileType , String abonentKey , int profileKey , String varName , byte valueType , byte timePolicy , int finalDate , int lifeTime , int intValue)
    {
        this.seqNum = seqNum;
        this.profileType = profileType;
        this.profileTypeFlag = true;
        this.abonentKey = abonentKey;
        this.abonentKeyFlag = true;
        this.profileKey = profileKey;
        this.profileKeyFlag = true;
        this.varName = varName;
        this.varNameFlag = true;
        this.valueType = valueType;
        this.valueTypeFlag = true;
        this.timePolicy = timePolicy;
        this.timePolicyFlag = true;
        this.finalDate = finalDate;
        this.finalDateFlag = true;
        this.lifeTime = lifeTime;
        this.lifeTimeFlag = true;
        this.intValue = intValue;
        this.intValueFlag = true;
    }
 
    public void clear()
    {
        profileTypeFlag=false;
        abonentKeyFlag=false;
        profileKeyFlag=false;
        varNameFlag=false;
        valueTypeFlag=false;
        timePolicyFlag=false;
        finalDateFlag=false;
        lifeTimeFlag=false;
        intValueFlag=false;
    }
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("PC_INC:");
        sb.append("seqNum=");
        sb.append(seqNum);
        if (profileTypeFlag) {
            sb.append(";profileType=");
            sb.append(profileType);
        }
        if (abonentKeyFlag) {
            sb.append(";abonentKey=");
            sb.append(abonentKey);
        }
        if (profileKeyFlag) {
            sb.append(";profileKey=");
            sb.append(profileKey);
        }
        if (varNameFlag) {
            sb.append(";varName=");
            sb.append(varName);
        }
        if (valueTypeFlag) {
            sb.append(";valueType=");
            sb.append(valueType);
        }
        if (timePolicyFlag) {
            sb.append(";timePolicy=");
            sb.append(timePolicy);
        }
        if (finalDateFlag) {
            sb.append(";finalDate=");
            sb.append(finalDate);
        }
        if (lifeTimeFlag) {
            sb.append(";lifeTime=");
            sb.append(lifeTime);
        }
        if (intValueFlag) {
            sb.append(";intValue=");
            sb.append(intValue);
        }
        return sb.toString();
    }

    public byte getProfileType()
           throws FieldIsNullException
    {
        if(!profileTypeFlag)
        {
            throw new FieldIsNullException("profileType");
        }
        return profileType;
    }

    public void setProfileType(byte profileType)
    {
        this.profileType = profileType;
        this.profileTypeFlag = true;
    }

    public boolean hasProfileType()
    {
        return profileTypeFlag;
    }

    public String getAbonentKey()
           throws FieldIsNullException
    {
        if(!abonentKeyFlag)
        {
            throw new FieldIsNullException("abonentKey");
        }
        return abonentKey;
    }

    public void setAbonentKey(String abonentKey)
    {
        this.abonentKey = abonentKey;
        this.abonentKeyFlag = true;
    }

    public boolean hasAbonentKey()
    {
        return abonentKeyFlag;
    }

    public int getProfileKey()
           throws FieldIsNullException
    {
        if(!profileKeyFlag)
        {
            throw new FieldIsNullException("profileKey");
        }
        return profileKey;
    }

    public void setProfileKey(int profileKey)
    {
        this.profileKey = profileKey;
        this.profileKeyFlag = true;
    }

    public boolean hasProfileKey()
    {
        return profileKeyFlag;
    }

    public String getVarName()
           throws FieldIsNullException
    {
        if(!varNameFlag)
        {
            throw new FieldIsNullException("varName");
        }
        return varName;
    }

    public void setVarName(String varName)
    {
        this.varName = varName;
        this.varNameFlag = true;
    }

    public boolean hasVarName()
    {
        return varNameFlag;
    }

    public byte getValueType()
           throws FieldIsNullException
    {
        if(!valueTypeFlag)
        {
            throw new FieldIsNullException("valueType");
        }
        return valueType;
    }

    public void setValueType(byte valueType)
    {
        this.valueType = valueType;
        this.valueTypeFlag = true;
    }

    public boolean hasValueType()
    {
        return valueTypeFlag;
    }

    public byte getTimePolicy()
           throws FieldIsNullException
    {
        if(!timePolicyFlag)
        {
            throw new FieldIsNullException("timePolicy");
        }
        return timePolicy;
    }

    public void setTimePolicy(byte timePolicy)
    {
        this.timePolicy = timePolicy;
        this.timePolicyFlag = true;
    }

    public boolean hasTimePolicy()
    {
        return timePolicyFlag;
    }

    public int getFinalDate()
           throws FieldIsNullException
    {
        if(!finalDateFlag)
        {
            throw new FieldIsNullException("finalDate");
        }
        return finalDate;
    }

    public void setFinalDate(int finalDate)
    {
        this.finalDate = finalDate;
        this.finalDateFlag = true;
    }

    public boolean hasFinalDate()
    {
        return finalDateFlag;
    }

    public int getLifeTime()
           throws FieldIsNullException
    {
        if(!lifeTimeFlag)
        {
            throw new FieldIsNullException("lifeTime");
        }
        return lifeTime;
    }

    public void setLifeTime(int lifeTime)
    {
        this.lifeTime = lifeTime;
        this.lifeTimeFlag = true;
    }

    public boolean hasLifeTime()
    {
        return lifeTimeFlag;
    }

    public int getIntValue()
           throws FieldIsNullException
    {
        if(!intValueFlag)
        {
            throw new FieldIsNullException("intValue");
        }
        return intValue;
    }

    public void setIntValue(int intValue)
    {
        this.intValue = intValue;
        this.intValueFlag = true;
    }

    public boolean hasIntValue()
    {
        return intValueFlag;
    }

    public void encode( PVAP proto, IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        System.out.println("write pos=" + writer.getPos() + " field=" + profileTypeTag);
        writer.writeTag(profileTypeTag);
        writer.writeByteLV(profileType);
        System.out.println("write pos=" + writer.getPos() + " field=" + varNameTag);
        writer.writeTag(varNameTag);
        writer.writeUTFLV(varName);
        System.out.println("write pos=" + writer.getPos() + " field=" + valueTypeTag);
        writer.writeTag(valueTypeTag);
        writer.writeByteLV(valueType);
        System.out.println("write pos=" + writer.getPos() + " field=" + timePolicyTag);
        writer.writeTag(timePolicyTag);
        writer.writeByteLV(timePolicy);
        System.out.println("write pos=" + writer.getPos() + " field=" + finalDateTag);
        writer.writeTag(finalDateTag);
        writer.writeIntLV(finalDate);
        System.out.println("write pos=" + writer.getPos() + " field=" + lifeTimeTag);
        writer.writeTag(lifeTimeTag);
        writer.writeIntLV(lifeTime);
        System.out.println("write pos=" + writer.getPos() + " field=" + intValueTag);
        writer.writeTag(intValueTag);
        writer.writeIntLV(intValue);
        // optional fields
        if (abonentKeyFlag) {
            System.out.println("write pos=" + writer.getPos() + " field=" + abonentKeyTag);
            writer.writeTag(abonentKeyTag);
            writer.writeUTFLV(abonentKey);
        }
        if (profileKeyFlag) {
            System.out.println("write pos=" + writer.getPos() + " field=" + profileKeyTag);
            writer.writeTag(profileKeyTag);
            writer.writeIntLV(profileKey);
        }
    }

    public void decode( PVAP proto, IBufferReader reader ) throws java.io.IOException
    {
        clear();
        while( true ) {
            int pos = reader.getPos();
            int tag = reader.readTag();
            System.out.println("read pos=" + pos + " field=" + tag);
            if ( tag == -1 ) break;
            switch( tag ) {
            case profileTypeTag: {
                if (profileTypeFlag) {
                    throw new DuplicateFieldException("profileType");
                }
                profileType=reader.readByteLV();
                profileTypeFlag=true;
                break;
            }
            case abonentKeyTag: {
                if (abonentKeyFlag) {
                    throw new DuplicateFieldException("abonentKey");
                }
                abonentKey=reader.readUTFLV();
                abonentKeyFlag=true;
                break;
            }
            case profileKeyTag: {
                if (profileKeyFlag) {
                    throw new DuplicateFieldException("profileKey");
                }
                profileKey=reader.readIntLV();
                profileKeyFlag=true;
                break;
            }
            case varNameTag: {
                if (varNameFlag) {
                    throw new DuplicateFieldException("varName");
                }
                varName=reader.readUTFLV();
                varNameFlag=true;
                break;
            }
            case valueTypeTag: {
                if (valueTypeFlag) {
                    throw new DuplicateFieldException("valueType");
                }
                valueType=reader.readByteLV();
                valueTypeFlag=true;
                break;
            }
            case timePolicyTag: {
                if (timePolicyFlag) {
                    throw new DuplicateFieldException("timePolicy");
                }
                timePolicy=reader.readByteLV();
                timePolicyFlag=true;
                break;
            }
            case finalDateTag: {
                if (finalDateFlag) {
                    throw new DuplicateFieldException("finalDate");
                }
                finalDate=reader.readIntLV();
                finalDateFlag=true;
                break;
            }
            case lifeTimeTag: {
                if (lifeTimeFlag) {
                    throw new DuplicateFieldException("lifeTime");
                }
                lifeTime=reader.readIntLV();
                lifeTimeFlag=true;
                break;
            }
            case intValueTag: {
                if (intValueFlag) {
                    throw new DuplicateFieldException("intValue");
                }
                intValue=reader.readIntLV();
                intValueFlag=true;
                break;
            }
            default:
                throw new NotImplementedException("reaction of reading unknown");
            }
        }
        checkFields();
    }

    public int getSeqNum()
    {
        return seqNum;
    }

    public void setSeqNum(int seqNum)
    {
        this.seqNum = seqNum;
    }

    protected void checkFields() throws MandatoryFieldMissingException
    {
        // checking mandatory fields
        if (!profileTypeFlag) {
            throw new MandatoryFieldMissingException("profileType");
        }
        if (!varNameFlag) {
            throw new MandatoryFieldMissingException("varName");
        }
        if (!valueTypeFlag) {
            throw new MandatoryFieldMissingException("valueType");
        }
        if (!timePolicyFlag) {
            throw new MandatoryFieldMissingException("timePolicy");
        }
        if (!finalDateFlag) {
            throw new MandatoryFieldMissingException("finalDate");
        }
        if (!lifeTimeFlag) {
            throw new MandatoryFieldMissingException("lifeTime");
        }
        if (!intValueFlag) {
            throw new MandatoryFieldMissingException("intValue");
        }
        // checking optional fields
        if (!abonentKeyFlag
            && (profileType==1)
            ) {
            throw new MandatoryFieldMissingException("abonentKey");
        }
        if (!profileKeyFlag
            && (profileType>1)
            && (profileType<5)
            ) {
            throw new MandatoryFieldMissingException("profileKey");
        }
    }
}
