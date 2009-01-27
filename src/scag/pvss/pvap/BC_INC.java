package pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class BC_INC  extends BC_CMD 
{
    // static Logger logger = Logger.getLogger(BC_INC.class);

    static final short varNameTag = 5;
    static final short valueTypeTag = 6;
    static final short timePolicyTag = 7;
    static final short finalDateTag = 8;
    static final short lifeTimeTag = 9;
    static final short intValueTag = 10;

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

    public BC_INC() {
    }

    public BC_INC(int seqNum, String varName , byte valueType , byte timePolicy , int finalDate , int lifeTime , int intValue)
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
    }
 
    public void clear()
    {
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
        sb.append("BC_INC:");
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
        return sb.toString();
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

    public void encode( IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
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
        // optional fields
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
            default:
                System.err.println("unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + getClass().getName());
                // logger.warn( "unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + BC_INC.class.getName() );
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
        // checking optional fields
    }
}
