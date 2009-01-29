package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class BC_INC_MOD  extends BC_CMD 
{
    // static Logger logger = Logger.getLogger(BC_INC_MOD.class);

    static final int varNameTag = 5;
    static final int valueTypeTag = 6;
    static final int timePolicyTag = 7;
    static final int finalDateTag = 8;
    static final int lifeTimeTag = 9;
    static final int intValueTag = 10;
    static final int modValueTag = 14;

    int seqNum;
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

    public BC_INC_MOD() {
    }

    public BC_INC_MOD(int seqNum, String varName , byte valueType , byte timePolicy , int finalDate , int lifeTime , int intValue , int modValue)
    {
        this.seqNum = seqNum;
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
        sb.append("BC_INC_MOD:");
        sb.append("seqNum=");
        sb.append(seqNum);
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

    public int getModValue()
           throws FieldIsNullException
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

    public void encode( PVAPBC proto, IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
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
        System.out.println("write pos=" + writer.getPos() + " field=" + modValueTag);
        writer.writeTag(modValueTag);
        writer.writeIntLV(modValue);
        // optional fields
    }

    public void decode( PVAPBC proto, IBufferReader reader ) throws java.io.IOException
    {
        clear();
        while( true ) {
            int pos = reader.getPos();
            int tag = reader.readTag();
            System.out.println("read pos=" + pos + " field=" + tag);
            if ( tag == -1 ) break;
            switch( tag ) {
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
            case modValueTag: {
                if (modValueFlag) {
                    throw new DuplicateFieldException("modValue");
                }
                modValue=reader.readIntLV();
                modValueFlag=true;
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
    }
}
