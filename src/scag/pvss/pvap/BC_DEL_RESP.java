package pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class BC_DEL_RESP  extends BC_CMD 
{
    // static Logger logger = Logger.getLogger(BC_DEL_RESP.class);

    static final short statusTag = 1;

    int seqNum;
    byte status;
    boolean statusFlag=false;

    public BC_DEL_RESP() {
    }

    public BC_DEL_RESP(int seqNum, byte status)
    {
        this.seqNum = seqNum;
        this.status = status;
        this.statusFlag = true;
    }
 
    public void clear()
    {
        statusFlag=false;
    }
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("BC_DEL_RESP:");
        sb.append("seqNum=");
        sb.append(seqNum);
        if (statusFlag) {
            sb.append(";status=");
            sb.append(status);
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

    public void encode( IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        writer.writeTag(statusTag);
        writer.writeByteLV(status);
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
            default:
                System.err.println("unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + getClass().getName());
                // logger.warn( "unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + BC_DEL_RESP.class.getName() );
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
        // checking optional fields
    }
}
