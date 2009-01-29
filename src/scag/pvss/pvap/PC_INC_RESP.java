package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_INC_RESP  
{
    // static Logger logger = Logger.getLogger(PC_INC_RESP.class);

    static final int statusTag = 1;
    static final int intValueTag = 10;

    int seqNum;
    byte status;
    boolean statusFlag=false;
    int intValue;
    boolean intValueFlag=false;

    public PC_INC_RESP() {
    }

    public PC_INC_RESP(int seqNum, byte status , int intValue)
    {
        this.seqNum = seqNum;
        this.status = status;
        this.statusFlag = true;
        this.intValue = intValue;
        this.intValueFlag = true;
    }
 
    public void clear()
    {
        statusFlag=false;
        intValueFlag=false;
    }
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("PC_INC_RESP:");
        sb.append("seqNum=");
        sb.append(seqNum);
        if (statusFlag) {
            sb.append(";status=");
            sb.append(status);
        }
        if (intValueFlag) {
            sb.append(";intValue=");
            sb.append(intValue);
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
        System.out.println("write pos=" + writer.getPos() + " field=" + statusTag);
        writer.writeTag(statusTag);
        writer.writeByteLV(status);
        System.out.println("write pos=" + writer.getPos() + " field=" + intValueTag);
        writer.writeTag(intValueTag);
        writer.writeIntLV(intValue);
        // optional fields
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
        if (!statusFlag) {
            throw new MandatoryFieldMissingException("status");
        }
        if (!intValueFlag) {
            throw new MandatoryFieldMissingException("intValue");
        }
        // checking optional fields
    }
}
