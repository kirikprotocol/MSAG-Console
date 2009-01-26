package pvss.pvap;

import protogen.framework.BufferWriter;
import protogen.framework.BufferReader;

import java.io.IOException;

import org.apache.log4j.Logger;


public class BC_SET {
  static Logger logger = Logger.getLogger(BC_SET.class);

  int seqNum;
  String varName;
  boolean varNameFlag=false;
  byte valueType;
  boolean valueTypeFlag=false;
  byte timePolicy;
  boolean timePolicyFlag=false;
  int finalDate;
  boolean finalDateFlag=false;
  int lifeTime;
  boolean lifeTimeFlag=false;
  int intValue;
  boolean intValueFlag=false;
  String stringValue;
  boolean stringValueFlag=false;
  byte boolValue;
  boolean boolValueFlag=false;
  int dateValue;
  boolean dateValueFlag=false;

  public BC_SET() {
  }

  public BC_SET(int seqNum,String varName, byte valueType, byte timePolicy, int finalDate, int lifeTime, int intValue, String stringValue, byte boolValue, int dateValue)
  {
    this.seqNum = seqNum;
    this.varName = varName;
    this.varNameFlag = true;
    this.valueType = valueType;
    this.valueTypeFlag = true;
    this.timePolicy = timePolicy;
    this.timePolicyFlag = true;
    this.finalDate = finalDate;
    this.finalDateFlag = true;
    this.lifeTime = lifeTime;
    this.lifeTimeFlag = true;
    this.intValue = intValue;
    this.intValueFlag = true;
    this.stringValue = stringValue;
    this.stringValueFlag = true;
    this.boolValue = boolValue;
    this.boolValueFlag = true;
    this.dateValue = dateValue;
    this.dateValueFlag = true;
  }
 
  public void clear()
  {
    varNameFlag=false;
    valueTypeFlag=false;
    timePolicyFlag=false;
    finalDateFlag=false;
    lifeTimeFlag=false;
    intValueFlag=false;
    stringValueFlag=false;
    boolValueFlag=false;
    dateValueFlag=false;
  }
 
  public String toString()
  {
    StringBuilder sb=new StringBuilder();
    sb.append("BC_SET:");
    sb.append("seqNum=");
    sb.append(seqNum);
    if(varNameFlag)
    {
      sb.append(";varName=");
      sb.append(varName.toString());    }
    if(valueTypeFlag)
    {
      sb.append(";valueType=");
      sb.append(valueType.toString());    }
    if(timePolicyFlag)
    {
      sb.append(";timePolicy=");
      sb.append(timePolicy.toString());    }
    if(finalDateFlag)
    {
      sb.append(";finalDate=");
      sb.append(finalDate);    }
    if(lifeTimeFlag)
    {
      sb.append(";lifeTime=");
      sb.append(lifeTime);    }
    if(intValueFlag)
    {
      sb.append(";intValue=");
      sb.append(intValue);    }
    if(stringValueFlag)
    {
      sb.append(";stringValue=");
      sb.append(stringValue.toString());    }
    if(boolValueFlag)
    {
      sb.append(";boolValue=");
      sb.append(boolValue.toString());    }
    if(dateValueFlag)
    {
      sb.append(";dateValue=");
      sb.append(dateValue);    }
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
  public byte getValueType()
  {
    if(!valueTypeFlag)
    {
      //!!TODO!!
    }
    return valueType;
  }

  public void setValueType(byte valueType)
  {
    this.valueType = valueType;
    this.valueTypeFlag = true;
  }
 
  public boolean hasValueType()
  {
    return valueTypeFlag;
  }
  public byte getTimePolicy()
  {
    if(!timePolicyFlag)
    {
      //!!TODO!!
    }
    return timePolicy;
  }

  public void setTimePolicy(byte timePolicy)
  {
    this.timePolicy = timePolicy;
    this.timePolicyFlag = true;
  }
 
  public boolean hasTimePolicy()
  {
    return timePolicyFlag;
  }
  public int getFinalDate()
  {
    if(!finalDateFlag)
    {
      //!!TODO!!
    }
    return finalDate;
  }

  public void setFinalDate(int finalDate)
  {
    this.finalDate = finalDate;
    this.finalDateFlag = true;
  }
 
  public boolean hasFinalDate()
  {
    return finalDateFlag;
  }
  public int getLifeTime()
  {
    if(!lifeTimeFlag)
    {
      //!!TODO!!
    }
    return lifeTime;
  }

  public void setLifeTime(int lifeTime)
  {
    this.lifeTime = lifeTime;
    this.lifeTimeFlag = true;
  }
 
  public boolean hasLifeTime()
  {
    return lifeTimeFlag;
  }
  public int getIntValue()
  {
    if(!intValueFlag)
    {
      //!!TODO!!
    }
    return intValue;
  }

  public void setIntValue(int intValue)
  {
    this.intValue = intValue;
    this.intValueFlag = true;
  }
 
  public boolean hasIntValue()
  {
    return intValueFlag;
  }
  public String getStringValue()
  {
    if(!stringValueFlag)
    {
      //!!TODO!!
    }
    return stringValue;
  }

  public void setStringValue(String stringValue)
  {
    this.stringValue = stringValue;
    this.stringValueFlag = true;
  }
 
  public boolean hasStringValue()
  {
    return stringValueFlag;
  }
  public byte getBoolValue()
  {
    if(!boolValueFlag)
    {
      //!!TODO!!
    }
    return boolValue;
  }

  public void setBoolValue(byte boolValue)
  {
    this.boolValue = boolValue;
    this.boolValueFlag = true;
  }
 
  public boolean hasBoolValue()
  {
    return boolValueFlag;
  }
  public int getDateValue()
  {
    if(!dateValueFlag)
    {
      //!!TODO!!
    }
    return dateValue;
  }

  public void setDateValue(int dateValue)
  {
    this.dateValue = dateValue;
    this.dateValueFlag = true;
  }
 
  public boolean hasDateValue()
  {
    return dateValueFlag;
  }

  public void encode(BufferWriter writer)
  {
    if(!varNameFlag)
    {
      //!!TODO!!
    }
    if(!valueTypeFlag)
    {
      //!!TODO!!
    }
    if(!timePolicyFlag)
    {
      //!!TODO!!
    }
    if(!finalDateFlag)
    {
      //!!TODO!!
    }
    if(!lifeTimeFlag)
    {
      //!!TODO!!
    }
    writer.writeInt( seqNum );
 
    writer.writeShort((short)5); // tag id
    writer.writeStringLV(varName);
 
    writer.writeShort((short)6); // tag id
    writer.writeByteLV(valueType);
 
    writer.writeShort((short)7); // tag id
    writer.writeByteLV(timePolicy);
 
    writer.writeShort((short)8); // tag id
    writer.writeIntLV(finalDate);
 
    writer.writeShort((short)9); // tag id
    writer.writeIntLV(lifeTime);
    if(intValueFlag)
    { 
      writer.writeShort((short)10); // tag id
      writer.writeIntLV(intValue);
    }
    if(stringValueFlag)
    { 
      writer.writeShort((short)11); // tag id
      writer.writeStringLV(stringValue);
    }
    if(boolValueFlag)
    { 
      writer.writeShort((short)12); // tag id
      writer.writeByteLV(boolValue);
    }
    if(dateValueFlag)
    { 
      writer.writeShort((short)13); // tag id
      writer.writeIntLV(dateValue);
    }
    writer.writeShort((short)0xFFFF); // end message tag
  }

  public void decode(BufferReader reader) throws IOException
  {
    seqNum = reader.readInt();
    while( true ) {
      int tag = reader.readShort();
      if( tag == (short)0xFFFF ) break;
      switch( tag ) {
        case 5:{
          varName = reader.readStringLV();          varNameFlag=true;
          }break;
        case 6:{
          valueType = reader.readByteLV();          valueTypeFlag=true;
          }break;
        case 7:{
          timePolicy = reader.readByteLV();          timePolicyFlag=true;
          }break;
        case 8:{
          finalDate = reader.readIntLV();          finalDateFlag=true;
          }break;
        case 9:{
          lifeTime = reader.readIntLV();          lifeTimeFlag=true;
          }break;
        case 10:{
          intValue = reader.readIntLV();          intValueFlag=true;
          }break;
        case 11:{
          stringValue = reader.readStringLV();          stringValueFlag=true;
          }break;
        case 12:{
          boolValue = reader.readByteLV();          boolValueFlag=true;
          }break;
        case 13:{
          dateValue = reader.readIntLV();          dateValueFlag=true;
          }break;
        default:
          logger.warn("unknown tagId: "+tag+" seqnum: "+seqNum+" msg: "+BC_SET.class.getName());
      }

    }
  }
}
