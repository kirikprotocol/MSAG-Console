package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_GET_RESP  
{
    // static Logger logger = Logger.getLogger(PC_GET_RESP.class);

    static final int statusTag = 1;
    static final int valueTypeTag = 6;
    static final int timePolicyTag = 7;
    static final int finalDateTag = 8;
    static final int lifeTimeTag = 9;
    static final int varNameTag = 5;
    static final int intValueTag = 10;
    static final int stringValueTag = 11;
    static final int boolValueTag = 12;
    static final int dateValueTag = 13;

    int seqNum;
    byte status;
    boolean statusFlag=false;
    byte valueType;
    boolean valueTypeFlag=false;
    byte timePolicy;
    boolean timePolicyFlag=false;
    int finalDate;
    boolean finalDateFlag=false;
    int lifeTime;
    boolean lifeTimeFlag=false;
    String varName;
    boolean varNameFlag=false;
    int intValue;
    boolean intValueFlag=false;
    String stringValue;
    boolean stringValueFlag=false;
    byte boolValue;
    boolean boolValueFlag=false;
    int dateValue;
    boolean dateValueFlag=false;

    public PC_GET_RESP() {
    }

    public PC_GET_RESP(int seqNum, byte status , byte valueType , byte timePolicy , int finalDate , int lifeTime , String varName , int intValue , String stringValue , byte boolValue , int dateValue)
    {
        this.seqNum = seqNum;
        this.status = status;
        this.statusFlag = true;
        this.valueType = valueType;
        this.valueTypeFlag = true;
        this.timePolicy = timePolicy;
        this.timePolicyFlag = true;
        this.finalDate = finalDate;
        this.finalDateFlag = true;
        this.lifeTime = lifeTime;
        this.lifeTimeFlag = true;
        this.varName = varName;
        this.varNameFlag = true;
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
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("PC_GET_RESP:");
        sb.append("seqNum=");
        sb.append(seqNum);
        if (statusFlag) {
            sb.append(";status=");
            sb.append(status);
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
        if (varNameFlag) {
            sb.append(";varName=");
            sb.append(varName);
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

    public byte getStatus()
           throws FieldIsNullException
    {
        if(!statusFlag)
        {
            throw new FieldIsNullException("status");
        }
        return status;
    }

    public void setStatus(byte status)
    {
        this.status = status;
        this.statusFlag = true;
    }

    public boolean hasStatus()
    {
        return statusFlag;
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

    public String getStringValue()
           throws FieldIsNullException
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

    public byte getBoolValue()
           throws FieldIsNullException
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

    public int getDateValue()
           throws FieldIsNullException
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

    public void encode( PVAP proto, IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        System.out.println("write pos=" + writer.getPos() + " field=" + statusTag);
        writer.writeTag(statusTag);
        writer.writeByteLV(status);
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
        System.out.println("write pos=" + writer.getPos() + " field=" + varNameTag);
        writer.writeTag(varNameTag);
        writer.writeUTFLV(varName);
        // optional fields
        if (intValueFlag) {
            System.out.println("write pos=" + writer.getPos() + " field=" + intValueTag);
            writer.writeTag(intValueTag);
            writer.writeIntLV(intValue);
        }
        if (stringValueFlag) {
            System.out.println("write pos=" + writer.getPos() + " field=" + stringValueTag);
            writer.writeTag(stringValueTag);
            writer.writeUTFLV(stringValue);
        }
        if (boolValueFlag) {
            System.out.println("write pos=" + writer.getPos() + " field=" + boolValueTag);
            writer.writeTag(boolValueTag);
            writer.writeByteLV(boolValue);
        }
        if (dateValueFlag) {
            System.out.println("write pos=" + writer.getPos() + " field=" + dateValueTag);
            writer.writeTag(dateValueTag);
            writer.writeIntLV(dateValue);
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
            case statusTag: {
                if (statusFlag) {
                    throw new DuplicateFieldException("status");
                }
                status=reader.readByteLV();
                statusFlag=true;
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
            case varNameTag: {
                if (varNameFlag) {
                    throw new DuplicateFieldException("varName");
                }
                varName=reader.readUTFLV();
                varNameFlag=true;
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
            case stringValueTag: {
                if (stringValueFlag) {
                    throw new DuplicateFieldException("stringValue");
                }
                stringValue=reader.readUTFLV();
                stringValueFlag=true;
                break;
            }
            case boolValueTag: {
                if (boolValueFlag) {
                    throw new DuplicateFieldException("boolValue");
                }
                boolValue=reader.readByteLV();
                boolValueFlag=true;
                break;
            }
            case dateValueTag: {
                if (dateValueFlag) {
                    throw new DuplicateFieldException("dateValue");
                }
                dateValue=reader.readIntLV();
                dateValueFlag=true;
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
        if (!statusFlag) {
            throw new MandatoryFieldMissingException("status");
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
        if (!varNameFlag) {
            throw new MandatoryFieldMissingException("varName");
        }
        // checking optional fields
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
