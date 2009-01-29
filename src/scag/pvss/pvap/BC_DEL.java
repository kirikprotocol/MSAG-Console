package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class BC_DEL  extends BC_CMD 
{
    // static Logger logger = Logger.getLogger(BC_DEL.class);

    static final int varNameTag = 5;

    int seqNum;
    String varName;
    boolean varNameFlag=false;

    public BC_DEL() {
    }

    public BC_DEL(int seqNum, String varName)
    {
        this.seqNum = seqNum;
        this.varName = varName;
        this.varNameFlag = true;
    }
 
    public void clear()
    {
        varNameFlag=false;
    }
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("BC_DEL:");
        sb.append("seqNum=");
        sb.append(seqNum);
        if (varNameFlag) {
            sb.append(";varName=");
            sb.append(varName);
        }
        return sb.toString();
    }

    public String getVarName()
           throws FieldIsNullException
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

    public void encode( PVAPBC proto, IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        System.out.println("write pos=" + writer.getPos() + " field=" + varNameTag);
        writer.writeTag(varNameTag);
        writer.writeUTFLV(varName);
        // optional fields
    }

    public void decode( PVAPBC proto, IBufferReader reader ) throws java.io.IOException
    {
        clear();
        while( true ) {
            int pos = reader.getPos();
            int tag = reader.readTag();
            System.out.println("read pos=" + pos + " field=" + tag);
            if ( tag == -1 ) break;
            switch( tag ) {
            case varNameTag: {
                if (varNameFlag) {
                    throw new DuplicateFieldException("varName");
                }
                varName=reader.readUTFLV();
                varNameFlag=true;
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
        if (!varNameFlag) {
            throw new MandatoryFieldMissingException("varName");
        }
        // checking optional fields
    }
}
