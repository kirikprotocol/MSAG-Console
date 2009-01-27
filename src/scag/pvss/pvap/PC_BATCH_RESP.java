package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_BATCH_RESP  
{
    // static Logger logger = Logger.getLogger(PC_BATCH_RESP.class);

    static final short statusTag = 1;
    static final short countTag = 22;

    int seqNum;
    byte status;
    boolean statusFlag=false;
    short count;
    boolean countFlag=false;

    public PC_BATCH_RESP() {
    }

    public PC_BATCH_RESP(int seqNum, byte status , short count)
    {
        this.seqNum = seqNum;
        this.status = status;
        this.statusFlag = true;
        this.count = count;
        this.countFlag = true;
    }
 
    public void clear()
    {
        statusFlag=false;
        countFlag=false;
    }
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("PC_BATCH_RESP:");
        sb.append("seqNum=");
        sb.append(seqNum);
        if (statusFlag) {
            sb.append(";status=");
            sb.append(status);
        }
        if (countFlag) {
            sb.append(";count=");
            sb.append(count);
        }
        return sb.toString();
    }

    public byte getStatus() throws FieldIsNullException
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

    public short getCount() throws FieldIsNullException
    {
        if(!countFlag)
        {
            throw new FieldIsNullException("count");
        }
        return count;
    }

    public void setCount(short count)
    {
        this.count = count;
        this.countFlag = true;
    }

    public boolean hasCount()
    {
        return countFlag;
    }

    public void encode( IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        writer.writeTag(statusTag);
        writer.writeByteLV(status);
        writer.writeTag(countTag);
        writer.writeShortLV(count);
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
            case statusTag: {
                status=reader.readByteLV();
                statusFlag=true;
                break;
            }
            case countTag: {
                count=reader.readShortLV();
                countFlag=true;
                break;
            }
            default:
                System.err.println("unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + getClass().getName());
                // logger.warn( "unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + PC_BATCH_RESP.class.getName() );
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
        if (!countFlag) {
            throw new MandatoryFieldMissingException("count");
        }
        // checking optional fields
    }
}
