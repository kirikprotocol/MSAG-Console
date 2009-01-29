package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_PING  
{
    // static Logger logger = Logger.getLogger(PC_PING.class);


    int seqNum;

    public PC_PING() {
    }

    public PC_PING(int seqNum)
    {
        this.seqNum = seqNum;
    }
 
    public void clear()
    {
    }
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("PC_PING:");
        sb.append("seqNum=");
        sb.append(seqNum);
        return sb.toString();
    }

    public void encode( PVAP proto, IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
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
        // checking optional fields
    }
}
