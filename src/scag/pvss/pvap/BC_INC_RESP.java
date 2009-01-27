package pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class BC_INC_RESP  extends BC_CMD 
{
    // static Logger logger = Logger.getLogger(BC_INC_RESP.class);

    static final short statusTag = 1;
    static final short intValueTag = 10;

    int seqNum;
    byte status;
    boolean statusFlag=false;
    int intValue;
    boolean intValueFlag=false;

    public BC_INC_RESP() {
    }

    public BC_INC_RESP(int seqNum, byte status , int intValue)
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
        sb.append("BC_INC_RESP:");
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
        writer.writeTag(statusTag);
        writer.writeByteLV(status);
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
            case statusTag: {
                status=reader.readByteLV();
                statusFlag=true;
                break;
            }
            case intValueTag: {
                intValue=reader.readIntLV();
                intValueFlag=true;
                break;
            }
            default:
                System.err.println("unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + getClass().getName());
                // logger.warn( "unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + BC_INC_RESP.class.getName() );
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
