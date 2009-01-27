package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_AUTH_RESP  
{
    // static Logger logger = Logger.getLogger(PC_AUTH_RESP.class);

    static final short statusTag = 1;
    static final short clientTypeTag = 19;
    static final short sidTag = 20;

    int seqNum;
    byte status;
    boolean statusFlag=false;
    byte clientType;
    boolean clientTypeFlag=false;
    byte sid;
    boolean sidFlag=false;

    public PC_AUTH_RESP() {
    }

    public PC_AUTH_RESP(int seqNum, byte status , byte clientType , byte sid)
    {
        this.seqNum = seqNum;
        this.status = status;
        this.statusFlag = true;
        this.clientType = clientType;
        this.clientTypeFlag = true;
        this.sid = sid;
        this.sidFlag = true;
    }
 
    public void clear()
    {
        statusFlag=false;
        clientTypeFlag=false;
        sidFlag=false;
    }
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("PC_AUTH_RESP:");
        sb.append("seqNum=");
        sb.append(seqNum);
        if (statusFlag) {
            sb.append(";status=");
            sb.append(status);
        }
        if (clientTypeFlag) {
            sb.append(";clientType=");
            sb.append(clientType);
        }
        if (sidFlag) {
            sb.append(";sid=");
            sb.append(sid);
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

    public byte getClientType() throws FieldIsNullException
    {
        if(!clientTypeFlag)
        {
            throw new FieldIsNullException("clientType");
        }
        return clientType;
    }

    public void setClientType(byte clientType)
    {
        this.clientType = clientType;
        this.clientTypeFlag = true;
    }

    public boolean hasClientType()
    {
        return clientTypeFlag;
    }

    public byte getSid() throws FieldIsNullException
    {
        if(!sidFlag)
        {
            throw new FieldIsNullException("sid");
        }
        return sid;
    }

    public void setSid(byte sid)
    {
        this.sid = sid;
        this.sidFlag = true;
    }

    public boolean hasSid()
    {
        return sidFlag;
    }

    public void encode( IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        writer.writeTag(statusTag);
        writer.writeByteLV(status);
        // optional fields
        if (clientTypeFlag) {
            writer.writeTag(clientTypeTag);
            writer.writeByteLV(clientType);
        }
        if (sidFlag) {
            writer.writeTag(sidTag);
            writer.writeByteLV(sid);
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
            case statusTag: {
                status=reader.readByteLV();
                statusFlag=true;
                break;
            }
            case clientTypeTag: {
                clientType=reader.readByteLV();
                clientTypeFlag=true;
                break;
            }
            case sidTag: {
                sid=reader.readByteLV();
                sidFlag=true;
                break;
            }
            default:
                System.err.println("unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + getClass().getName());
                // logger.warn( "unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + PC_AUTH_RESP.class.getName() );
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
        if (!clientTypeFlag
            && (status==0)
            ) {
            throw new MandatoryFieldMissingException("clientType");
        }
        if (!sidFlag
            && (status==0)
            ) {
            throw new MandatoryFieldMissingException("sid");
        }
    }
}
