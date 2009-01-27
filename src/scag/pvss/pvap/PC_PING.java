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

    public void encode( IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
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
            default:
                System.err.println("unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + getClass().getName());
                // logger.warn( "unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + PC_PING.class.getName() );
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
