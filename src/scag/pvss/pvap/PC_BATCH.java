package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_BATCH  
{
    // static Logger logger = Logger.getLogger(PC_BATCH.class);

    static final int profileTypeTag = 2;
    static final int abonentKeyTag = 3;
    static final int profileKeyTag = 4;
    static final int batchModeTag = 21;
    static final int batchContentTag = 23;

    int seqNum;
    byte profileType;
    boolean profileTypeFlag=false;
    String abonentKey;
    boolean abonentKeyFlag=false;
    int profileKey;
    boolean profileKeyFlag=false;
    boolean batchMode;
    boolean batchModeFlag=false;
    BatchCmdArray batchContent;
    boolean batchContentFlag=false;

    public PC_BATCH() {
    }

    public PC_BATCH(int seqNum, byte profileType , String abonentKey , int profileKey , boolean batchMode , BatchCmdArray batchContent)
    {
        this.seqNum = seqNum;
        this.profileType = profileType;
        this.profileTypeFlag = true;
        this.abonentKey = abonentKey;
        this.abonentKeyFlag = true;
        this.profileKey = profileKey;
        this.profileKeyFlag = true;
        this.batchMode = batchMode;
        this.batchModeFlag = true;
        this.batchContent = batchContent;
        this.batchContentFlag = true;
    }
 
    public void clear()
    {
        profileTypeFlag=false;
        abonentKeyFlag=false;
        profileKeyFlag=false;
        batchModeFlag=false;
        batchContentFlag=false;
    }
 
    public String toString()
    {
        StringBuilder sb=new StringBuilder();
        sb.append("PC_BATCH:");
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
        if (batchModeFlag) {
            sb.append(";batchMode=");
            sb.append(batchMode);
        }
        if (batchContentFlag) {
            sb.append(";batchContent=");
            sb.append(batchContent.toString());
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

    public boolean getBatchMode()
           throws FieldIsNullException
    {
        if(!batchModeFlag)
        {
            throw new FieldIsNullException("batchMode");
        }
        return batchMode;
    }

    public void setBatchMode(boolean batchMode)
    {
        this.batchMode = batchMode;
        this.batchModeFlag = true;
    }

    public boolean hasBatchMode()
    {
        return batchModeFlag;
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
        System.out.println("write pos=" + writer.getPos() + " field=" + profileTypeTag);
        writer.writeTag(profileTypeTag);
        writer.writeByteLV(profileType);
        System.out.println("write pos=" + writer.getPos() + " field=" + batchModeTag);
        writer.writeTag(batchModeTag);
        writer.writeBoolLV(batchMode);
        System.out.println("write pos=" + writer.getPos() + " field=" + batchContentTag);
        writer.writeTag(batchContentTag);
        batchContent.encode(proto,writer);
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
            case batchModeTag: {
                if (batchModeFlag) {
                    throw new DuplicateFieldException("batchMode");
                }
                batchMode=reader.readBoolLV();
                batchModeFlag=true;
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
        if (!profileTypeFlag) {
            throw new MandatoryFieldMissingException("profileType");
        }
        if (!batchModeFlag) {
            throw new MandatoryFieldMissingException("batchMode");
        }
        if (!batchContentFlag) {
            throw new MandatoryFieldMissingException("batchContent");
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
