package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_AUTH_RESP  
{
    // static Logger logger = Logger.getLogger(PC_AUTH_RESP.class);

    static final int statusTag = 1;
    static final int clientTypeTag = 19;
    static final int sidTag = 20;

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

    public byte getClientType()
           throws FieldIsNullException
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

    public byte getSid()
           throws FieldIsNullException
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

    public void encode( PVAP proto, IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        System.out.println("write pos=" + writer.getPos() + " field=" + statusTag);
        writer.writeTag(statusTag);
        writer.writeByteLV(status);
        // optional fields
        if (clientTypeFlag) {
            System.out.println("write pos=" + writer.getPos() + " field=" + clientTypeTag);
            writer.writeTag(clientTypeTag);
            writer.writeByteLV(clientType);
        }
        if (sidFlag) {
            System.out.println("write pos=" + writer.getPos() + " field=" + sidTag);
            writer.writeTag(sidTag);
            writer.writeByteLV(sid);
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
            case clientTypeTag: {
                if (clientTypeFlag) {
                    throw new DuplicateFieldException("clientType");
                }
                clientType=reader.readByteLV();
                clientTypeFlag=true;
                break;
            }
            case sidTag: {
                if (sidFlag) {
                    throw new DuplicateFieldException("sid");
                }
                sid=reader.readByteLV();
                sidFlag=true;
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
