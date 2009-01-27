package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_INC_MOD  
{
    // static Logger logger = Logger.getLogger(PC_INC_MOD.class);

    static final short profileTypeTag = 2;
    static final short abonentKeyTag = 3;
    static final short profileKeyTag = 4;
    static final short varNameTag = 5;
    static final short valueTypeTag = 6;
    static final short timePolicyTag = 7;
    static final short finalDateTag = 8;
    static final short lifeTimeTag = 9;
    static final short intValueTag = 10;
    static final short modValueTag = 14;

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
    int modValue;
    boolean modValueFlag=false;

    public PC_INC_MOD() {
    }

    public PC_INC_MOD(int seqNum, byte profileType , String abonentKey , int profileKey , String varName , byte valueType , byte timePolicy , int finalDate , int lifeTime , int intValue , int modValue)
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
        this.modValue = modValue;
        this.modValueFlag = true;
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
        modValueFlag=false;
    }
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("PC_INC_MOD:");
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
        if (modValueFlag) {
            sb.append(";modValue=");
            sb.append(modValue);
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

    public int getModValue() throws FieldIsNullException
    {
        if(!modValueFlag)
        {
            throw new FieldIsNullException("modValue");
        }
        return modValue;
    }

    public void setModValue(int modValue)
    {
        this.modValue = modValue;
        this.modValueFlag = true;
    }

    public boolean hasModValue()
    {
        return modValueFlag;
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
        writer.writeTag(intValueTag);
        writer.writeIntLV(intValue);
        writer.writeTag(modValueTag);
        writer.writeIntLV(modValue);
        // optional fields
        if (abonentKeyFlag) {
            writer.writeTag(abonentKeyTag);
            writer.writeStringLV(abonentKey);
        }
        if (profileKeyFlag) {
            writer.writeTag(profileKeyTag);
            writer.writeIntLV(profileKey);
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
            case modValueTag: {
                modValue=reader.readIntLV();
                modValueFlag=true;
                break;
            }
            default:
                System.err.println("unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + getClass().getName());
                // logger.warn( "unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + PC_INC_MOD.class.getName() );
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
        if (!modValueFlag) {
            throw new MandatoryFieldMissingException("modValue");
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
