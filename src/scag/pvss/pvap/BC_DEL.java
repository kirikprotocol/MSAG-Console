package pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class BC_DEL  extends BC_CMD 
{
    // static Logger logger = Logger.getLogger(BC_DEL.class);

    static final short varNameTag = 5;

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

    public String getVarName() throws FieldIsNullException
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

    public void encode( IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        writer.writeTag(varNameTag);
        writer.writeStringLV(varName);
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
            case varNameTag: {
                varName=reader.readStringLV();
                varNameFlag=true;
                break;
            }
            default:
                System.err.println("unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + getClass().getName());
                // logger.warn( "unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + BC_DEL.class.getName() );
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
