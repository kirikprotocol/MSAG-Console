package com.eyelinecom.whoisd.pvss.pvap;

// import protogen.framework.BufferWriter;
// import protogen.framework.BufferReader;
// import java.io.IOException;

// import org.apache.log4j.Logger;


public class PC_BATCH  
{
    // static Logger logger = Logger.getLogger(PC_BATCH.class);

    static final short profileTypeTag = 2;
    static final short abonentKeyTag = 3;
    static final short profileKeyTag = 4;
    static final short batchModeTag = 21;
    static final short countTag = 22;

    int seqNum;
    byte profileType;
    boolean profileTypeFlag=false;
    String abonentKey;
    boolean abonentKeyFlag=false;
    int profileKey;
    boolean profileKeyFlag=false;
    byte batchMode;
    boolean batchModeFlag=false;
    short count;
    boolean countFlag=false;

    public PC_BATCH() {
    }

    public PC_BATCH(int seqNum, byte profileType , String abonentKey , int profileKey , byte batchMode , short count)
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
        this.count = count;
        this.countFlag = true;
    }
 
    public void clear()
    {
        profileTypeFlag=false;
        abonentKeyFlag=false;
        profileKeyFlag=false;
        batchModeFlag=false;
        countFlag=false;
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
        if (countFlag) {
            sb.append(";count=");
            sb.append(count);
        }
        return sb.toString();
    }

    public byte getProfileType() throws FieldIsNullException
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

    public String getAbonentKey() throws FieldIsNullException
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

    public int getProfileKey() throws FieldIsNullException
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

    public byte getBatchMode() throws FieldIsNullException
    {
        if(!batchModeFlag)
        {
            throw new FieldIsNullException("batchMode");
        }
        return batchMode;
    }

    public void setBatchMode(byte batchMode)
    {
        this.batchMode = batchMode;
        this.batchModeFlag = true;
    }

    public boolean hasBatchMode()
    {
        return batchModeFlag;
    }

    public short getCount() throws FieldIsNullException
    {
        if(!countFlag)
        {
            throw new FieldIsNullException("count");
        }
        return count;
    }

    public void setCount(short count)
    {
        this.count = count;
        this.countFlag = true;
    }

    public boolean hasCount()
    {
        return countFlag;
    }

    public void encode( IBufferWriter writer ) throws java.io.IOException
    {
        checkFields();
        // mandatory fields
        writer.writeTag(profileTypeTag);
        writer.writeByteLV(profileType);
        writer.writeTag(batchModeTag);
        writer.writeByteLV(batchMode);
        writer.writeTag(countTag);
        writer.writeShortLV(count);
        // optional fields
        if (abonentKeyFlag) {
            writer.writeTag(abonentKeyTag);
            writer.writeStringLV(abonentKey);
        }
        if (profileKeyFlag) {
            writer.writeTag(profileKeyTag);
            writer.writeIntLV(profileKey);
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
            case profileTypeTag: {
                profileType=reader.readByteLV();
                profileTypeFlag=true;
                break;
            }
            case abonentKeyTag: {
                abonentKey=reader.readStringLV();
                abonentKeyFlag=true;
                break;
            }
            case profileKeyTag: {
                profileKey=reader.readIntLV();
                profileKeyFlag=true;
                break;
            }
            case batchModeTag: {
                batchMode=reader.readByteLV();
                batchModeFlag=true;
                break;
            }
            case countTag: {
                count=reader.readShortLV();
                countFlag=true;
                break;
            }
            default:
                System.err.println("unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + getClass().getName());
                // logger.warn( "unknown tagId: " + tag + " seqnum: " + seqNum + " msg: " + PC_BATCH.class.getName() );
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
        if (!countFlag) {
            throw new MandatoryFieldMissingException("count");
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
