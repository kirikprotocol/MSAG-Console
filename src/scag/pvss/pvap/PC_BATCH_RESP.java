package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_BATCH_RESP  
{
    // static Logger logger = Logger.getLogger(PC_BATCH_RESP.class);

    static final int statusTag = 1;
    static final int batchContentTag = 23;

    int seqNum;
    byte status;
    boolean statusFlag=false;
    BatchCmdArray batchContent;
    boolean batchContentFlag=false;

    public PC_BATCH_RESP() {
    }

    public PC_BATCH_RESP(int seqNum, byte status , BatchCmdArray batchContent)
    {
        this.seqNum = seqNum;
        this.status = status;
        this.statusFlag = true;
        this.batchContent = batchContent;
        this.batchContentFlag = true;
    }
 
    public void clear()
    {
        statusFlag=false;
        batchContentFlag=false;
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
        if (batchContentFlag) {
            sb.append(";batchContent=");
            sb.append(batchContent.toString());
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

    public BatchCmdArray getBatchContent()
           throws FieldIsNullException
    {
        if(!batchContentFlag)
        {
            throw new FieldIsNullException("batchContent");
        }
        return batchContent;
    }

    public void setBatchContent(BatchCmdArray batchContent)
    {
        this.batchContent = batchContent;
        this.batchContentFlag = true;
    }

    public boolean hasBatchContent()
    {
        return batchContentFlag;
    }

    public void encode( PVAP proto, IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        System.out.println("write pos=" + writer.getPos() + " field=" + statusTag);
        writer.writeTag(statusTag);
        writer.writeByteLV(status);
        System.out.println("write pos=" + writer.getPos() + " field=" + batchContentTag);
        writer.writeTag(batchContentTag);
        batchContent.encode(proto,writer);
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
            case batchContentTag: {
                if (batchContentFlag) {
                    throw new DuplicateFieldException("batchContent");
                }
                batchContent = new BatchCmdArray();
                batchContent.decode(proto,reader);
                batchContentFlag=true;
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
        if (!batchContentFlag) {
            throw new MandatoryFieldMissingException("batchContent");
        }
        // checking optional fields
    }
}
