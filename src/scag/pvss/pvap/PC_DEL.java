package pvss.pvap;

import protogen.framework.BufferWriter;
import protogen.framework.BufferReader;

import java.io.IOException;

import org.apache.log4j.Logger;


public class PC_DEL {
  static Logger logger = Logger.getLogger(PC_DEL.class);

  int seqNum;
  byte profileType;
  boolean profileTypeFlag=false;
  String abonentKey;
  boolean abonentKeyFlag=false;
  int profileKey;
  boolean profileKeyFlag=false;
  String varName;
  boolean varNameFlag=false;

  public PC_DEL() {
  }

  public PC_DEL(int seqNum,byte profileType, String abonentKey, int profileKey, String varName)
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
    sb.append("PC_DEL:");
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
    if(varNameFlag)
    {
      sb.append(";varName=");
      sb.append(varName.toString());    }
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
  public String getVarName()
  {
    if(!varNameFlag)
    {
      //!!TODO!!
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

  public void encode(BufferWriter writer)
  {
    if(!profileTypeFlag)
    {
      //!!TODO!!
    }
    if(!varNameFlag)
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
 
    writer.writeShort((short)5); // tag id
    writer.writeStringLV(varName);
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
        case 5:{
          varName = reader.readStringLV();          varNameFlag=true;
          }break;
        default:
          logger.warn("unknown tagId: "+tag+" seqnum: "+seqNum+" msg: "+PC_DEL.class.getName());
      }

    }
  }
}
