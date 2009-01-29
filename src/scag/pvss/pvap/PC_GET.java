package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_GET  
{
    // static Logger logger = Logger.getLogger(PC_GET.class);

    static final int profileTypeTag = 2;
    static final int abonentKeyTag = 3;
    static final int profileKeyTag = 4;
    static final int varNameTag = 5;

    int seqNum;
    byte profileType;
    boolean profileTypeFlag=false;
    String abonentKey;
    boolean abonentKeyFlag=false;
    int profileKey;
    boolean profileKeyFlag=false;
    String varName;
    boolean varNameFlag=false;

    public PC_GET() {
    }

    public PC_GET(int seqNum, byte profileType , String abonentKey , int profileKey , String varName)
    {
        this.seqNum = seqNum;
        this.profileType = profileType;
        this.profileTypeFlag = true;
        this.abonentKey = abonentKey;
        this.abonentKeyFlag = true;
        this.profileKey = profileKey;
        this.profileKeyFlag = true;
        this.varName = varName;
        this.varNameFlag = true;
    }
 
    public void clear()
    {
        profileTypeFlag=false;
        abonentKeyFlag=false;
        profileKeyFlag=false;
        varNameFlag=false;
    }
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("PC_GET:");
        sb.append("seqNum=");
        sb.append(seqNum);
        if (profileTypeFlag) {
            sb.append(";profileType=");
            sb.append(profileType);
        }
        if (abonentKeyFlag) {
            sb.append(";abonentKey=");
            sb.append(abonentKey);
        }
        if (profileKeyFlag) {
            sb.append(";profileKey=");
            sb.append(profileKey);
        }
        if (varNameFlag) {
            sb.append(";varName=");
            sb.append(varName);
        }
        return sb.toString();
    }

    public byte getProfileType()
           throws FieldIsNullException
    {
        if(!profileTypeFlag)
        {
            throw new FieldIsNullException("profileType");
        }
        return profileType;
    }

    public void setProfileType(byte profileType)
    {
        this.profileType = profileType;
        this.profileTypeFlag = true;
    }

    public boolean hasProfileType()
    {
        return profileTypeFlag;
    }

    public String getAbonentKey()
           throws FieldIsNullException
    {
        if(!abonentKeyFlag)
        {
            throw new FieldIsNullException("abonentKey");
        }
        return abonentKey;
    }

    public void setAbonentKey(String abonentKey)
    {
        this.abonentKey = abonentKey;
        this.abonentKeyFlag = true;
    }

    public boolean hasAbonentKey()
    {
        return abonentKeyFlag;
    }

    public int getProfileKey()
           throws FieldIsNullException
    {
        if(!profileKeyFlag)
        {
            throw new FieldIsNullException("profileKey");
        }
        return profileKey;
    }

    public void setProfileKey(int profileKey)
    {
        this.profileKey = profileKey;
        this.profileKeyFlag = true;
    }

    public boolean hasProfileKey()
    {
        return profileKeyFlag;
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

    public void encode( PVAP proto, IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        System.out.println("write pos=" + writer.getPos() + " field=" + profileTypeTag);
        writer.writeTag(profileTypeTag);
        writer.writeByteLV(profileType);
        System.out.println("write pos=" + writer.getPos() + " field=" + varNameTag);
        writer.writeTag(varNameTag);
        writer.writeUTFLV(varName);
        // optional fields
        if (abonentKeyFlag) {
            System.out.println("write pos=" + writer.getPos() + " field=" + abonentKeyTag);
            writer.writeTag(abonentKeyTag);
            writer.writeUTFLV(abonentKey);
        }
        if (profileKeyFlag) {
            System.out.println("write pos=" + writer.getPos() + " field=" + profileKeyTag);
            writer.writeTag(profileKeyTag);
            writer.writeIntLV(profileKey);
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
            case profileTypeTag: {
                if (profileTypeFlag) {
                    throw new DuplicateFieldException("profileType");
                }
                profileType=reader.readByteLV();
                profileTypeFlag=true;
                break;
            }
            case abonentKeyTag: {
                if (abonentKeyFlag) {
                    throw new DuplicateFieldException("abonentKey");
                }
                abonentKey=reader.readUTFLV();
                abonentKeyFlag=true;
                break;
            }
            case profileKeyTag: {
                if (profileKeyFlag) {
                    throw new DuplicateFieldException("profileKey");
                }
                profileKey=reader.readIntLV();
                profileKeyFlag=true;
                break;
            }
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
        if (!profileTypeFlag) {
            throw new MandatoryFieldMissingException("profileType");
        }
        if (!varNameFlag) {
            throw new MandatoryFieldMissingException("varName");
        }
        // checking optional fields
        if (!abonentKeyFlag
            && (profileType==1)
            ) {
            throw new MandatoryFieldMissingException("abonentKey");
        }
        if (!profileKeyFlag
            && (profileType>1)
            && (profileType<5)
            ) {
            throw new MandatoryFieldMissingException("profileKey");
        }
    }
}
