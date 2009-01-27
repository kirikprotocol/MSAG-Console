package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_SET  
{
    // static Logger logger = Logger.getLogger(PC_SET.class);

    static final short profileTypeTag = 2;
    static final short abonentKeyTag = 3;
    static final short profileKeyTag = 4;
    static final short varNameTag = 5;
    static final short valueTypeTag = 6;
    static final short timePolicyTag = 7;
    static final short finalDateTag = 8;
    static final short lifeTimeTag = 9;
    static final short intValueTag = 10;
    static final short stringValueTag = 11;
    static final short boolValueTag = 12;
    static final short dateValueTag = 13;

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
    String stringValue;
    boolean stringValueFlag=false;
    byte boolValue;
    boolean boolValueFlag=false;
    int dateValue;
    boolean dateValueFlag=false;

    public PC_SET() {
    }

    public PC_SET(int seqNum, byte profileType , String abonentKey , int profileKey , String varName , byte valueType , byte timePolicy , int finalDate , int lifeTime , int intValue , String stringValue , byte boolValue , int dateValue)
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
        this.stringValue = stringValue;
        this.stringValueFlag = true;
        this.boolValue = boolValue;
        this.boolValueFlag = true;
        this.dateValue = dateValue;
        this.dateValueFlag = true;
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
        stringValueFlag=false;
        boolValueFlag=false;
        dateValueFlag=false;
    }
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("PC_SET:");
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
        if (stringValueFlag) {
            sb.append(";stringValue=");
            sb.append(stringValue);
        }
        if (boolValueFlag) {
            sb.append(";boolValue=");
            sb.append(boolValue);
        }
        if (dateValueFlag) {
            sb.append(";dateValue=");
            sb.append(dateValue);
        }
        return sb.toString();
    }

    public byte getProfileType() throws FieldIsNullException
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

    public String getAbonentKey() throws FieldIsNullException
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

    public int getProfileKey() throws FieldIsNullException
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

    public String getVarName() throws FieldIsNullException
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

    public byte getValueType() throws FieldIsNullException
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

    public byte getTimePolicy() throws FieldIsNullException
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

    public int getFinalDate() throws FieldIsNullException
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

    public int getLifeTime() throws FieldIsNullException
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

    public int getIntValue() throws FieldIsNullException
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

    public String getStringValue() throws FieldIsNullException
    {
        if(!stringValueFlag)
        {
            throw new FieldIsNullException("stringValue");
        }
        return stringValue;
    }

    public void setStringValue(String stringValue)
    {
        this.stringValue = stringValue;
        this.stringValueFlag = true;
    }

    public boolean hasStringValue()
    {
        return stringValueFlag;
    }

    public byte getBoolValue() throws FieldIsNullException
    {
        if(!boolValueFlag)
        {
            throw new FieldIsNullException("boolValue");
        }
        return boolValue;
    }

    public void setBoolValue(byte boolValue)
    {
        this.boolValue = boolValue;
        this.boolValueFlag = true;
    }

    public boolean hasBoolValue()
    {
        return boolValueFlag;
    }

    public int getDateValue() throws FieldIsNullException
    {
        if(!dateValueFlag)
        {
            throw new FieldIsNullException("dateValue");
        }
        return dateValue;
    }

    public void setDateValue(int dateValue)
    {
        this.dateValue = dateValue;
        this.dateValueFlag = true;
    }

    public boolean hasDateValue()
    {
        return dateValueFlag;
    }

    public void encode( IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        writer.writeTag(profileTypeTag);
        writer.writeByteLV(profileType);
        writer.writeTag(varNameTag);
        writer.writeStringLV(varName);
        writer.writeTag(valueTypeTag);
        writer.writeByteLV(valueType);
        writer.writeTag(timePolicyTag);
        writer.writeByteLV(timePolicy);
        writer.writeTag(finalDateTag);
        writer.writeIntLV(finalDate);
        writer.writeTag(lifeTimeTag);
        writer.writeIntLV(lifeTime);
        // optional fields
        if (abonentKeyFlag) {
            writer.writeTag(abonentKeyTag);
            writer.writeStringLV(abonentKey);
        }
        if (profileKeyFlag) {
            writer.writeTag(profileKeyTag);
            writer.writeIntLV(profileKey);
        }
        if (intValueFlag) {
            writer.writeTag(intValueTag);
            writer.writeIntLV(intValue);
        }
        if (stringValueFlag) {
            writer.writeTag(stringValueTag);
            writer.writeStringLV(stringValue);
        }
        if (boolValueFlag) {
            writer.writeTag(boolValueTag);
            writer.writeByteLV(boolValue);
        }
        if (dateValueFlag) {
            writer.writeTag(dateValueTag);
            writer.writeIntLV(dateValue);
        }
    }

    public void decode( IBufferReader reader ) throws java.io.IOException
    {
        clear();
        // seqNum = reader.readInt();
        while( true ) {
            short tag = reader.readTag();
            // System.out.println("tag got:" + tag);
            if ( tag == (short)0xFFFF ) break;
            switch( tag ) {
            case profileTypeTag: {
                profileType=reader.readByteLV();
                profileTypeFlag=true;
                break;
            }
            case abonentKeyTag: {
                abonentKey=reader.readStringLV();
                abonentKeyFlag=true;
                break;
            }
            case profileKeyTag: {
                profileKey=reader.readIntLV();
                profileKeyFlag=true;
                break;
            }
            case varNameTag: {
                varName=reader.readStringLV();
                varNameFlag=true;
                break;
            }
            case valueTypeTag: {
                valueType=reader.readByteLV();
                valueTypeFlag=true;
                break;
            }
            case timePolicyTag: {
                timePolicy=reader.readByteLV();
                timePolicyFlag=true;
                break;
            }
            case finalDateTag: {
                finalDate=reader.readIntLV();
                finalDateFlag=true;
                break;
            }
            case lifeTimeTag: {
                lifeTime=reader.readIntLV();
                lifeTimeFlag=true;
                break;
            }
            case intValueTag: {
                intValue=reader.readIntLV();
                intValueFlag=true;
                break;
            }
            case stringValueTag: {
                stringValue=reader.readStringLV();
                stringValueFlag=true;
                break;
            }
            case boolValueTag: {
                boolValue=reader.readByteLV();
                boolValueFlag=true;
                break;
            }
            case dateValueTag: {
                dateValue=reader.readIntLV();
                dateValueFlag=true;
                break;
            }
            default:
                System.err.println("unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + getClass().getName());
                // logger.warn( "unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + PC_SET.class.getName() );
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
        if (!intValueFlag
            && (valueType==1)
            ) {
            throw new MandatoryFieldMissingException("intValue");
        }
        if (!stringValueFlag
            && (valueType==2)
            ) {
            throw new MandatoryFieldMissingException("stringValue");
        }
        if (!boolValueFlag
            && (valueType==3)
            ) {
            throw new MandatoryFieldMissingException("boolValue");
        }
        if (!dateValueFlag
            && (valueType==4)
            ) {
            throw new MandatoryFieldMissingException("dateValue");
        }
    }
}
