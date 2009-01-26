package pvss.pvap;

import protogen.framework.BufferWriter;
import protogen.framework.BufferReader;

import java.io.IOException;

import org.apache.log4j.Logger;


public class PC_BATCH {
  static Logger logger = Logger.getLogger(PC_BATCH.class);

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

  public PC_BATCH(int seqNum,byte profileType, String abonentKey, int profileKey, byte batchMode, short count)
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
    if(profileTypeFlag)
    {
      sb.append(";profileType=");
      sb.append(profileType.toString());    }
    if(abonentKeyFlag)
    {
      sb.append(";abonentKey=");
      sb.append(abonentKey.toString());    }
    if(profileKeyFlag)
    {
      sb.append(";profileKey=");
      sb.append(profileKey);    }
    if(batchModeFlag)
    {
      sb.append(";batchMode=");
      sb.append(batchMode.toString());    }
    if(countFlag)
    {
      sb.append(";count=");
      sb.append(count.toString());    }
    return sb.toString();
  }

  public int getSeqNum()
  {
    return seqNum;
  }

  public void setSeqNum(int seqNum)
  {
    this.seqNum = seqNum;
  }

  public byte getProfileType()
  {
    if(!profileTypeFlag)
    {
      //!!TODO!!
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
  {
    if(!abonentKeyFlag)
    {
      //!!TODO!!
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
  {
    if(!profileKeyFlag)
    {
      //!!TODO!!
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
  public byte getBatchMode()
  {
    if(!batchModeFlag)
    {
      //!!TODO!!
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
  public short getCount()
  {
    if(!countFlag)
    {
      //!!TODO!!
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

  public void encode(BufferWriter writer)
  {
    if(!profileTypeFlag)
    {
      //!!TODO!!
    }
    if(!batchModeFlag)
    {
      //!!TODO!!
    }
    if(!countFlag)
    {
      //!!TODO!!
    }
    writer.writeInt( seqNum );
 
    writer.writeShort((short)2); // tag id
    writer.writeByteLV(profileType);
    if(abonentKeyFlag)
    { 
      writer.writeShort((short)3); // tag id
      writer.writeStringLV(abonentKey);
    }
    if(profileKeyFlag)
    { 
      writer.writeShort((short)4); // tag id
      writer.writeIntLV(profileKey);
    }
 
    writer.writeShort((short)21); // tag id
    writer.writeByteLV(batchMode);
 
    writer.writeShort((short)22); // tag id
    writer.writeShortLV(count);
    writer.writeShort((short)0xFFFF); // end message tag
  }

  public void decode(BufferReader reader) throws IOException
  {
    seqNum = reader.readInt();
    while( true ) {
      int tag = reader.readShort();
      if( tag == (short)0xFFFF ) break;
      switch( tag ) {
        case 2:{
          profileType = reader.readByteLV();          profileTypeFlag=true;
          }break;
        case 3:{
          abonentKey = reader.readStringLV();          abonentKeyFlag=true;
          }break;
        case 4:{
          profileKey = reader.readIntLV();          profileKeyFlag=true;
          }break;
        case 21:{
          batchMode = reader.readByteLV();          batchModeFlag=true;
          }break;
        case 22:{
          count = reader.readShortLV();          countFlag=true;
          }break;
        default:
          logger.warn("unknown tagId: "+tag+" seqnum: "+seqNum+" msg: "+PC_BATCH.class.getName());
      }

    }
  }
}
