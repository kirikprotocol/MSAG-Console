package pvss.pvap;

import protogen.framework.BufferWriter;
import protogen.framework.BufferReader;

import java.io.IOException;

import org.apache.log4j.Logger;


public class BC_INC {
  static Logger logger = Logger.getLogger(BC_INC.class);

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

  public BC_INC() {
  }

  public BC_INC(int seqNum,String varName, byte valueType, byte timePolicy, int finalDate, int lifeTime, int intValue)
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
  }
 
  public void clear()
  {
    varNameFlag=false;
    valueTypeFlag=false;
    timePolicyFlag=false;
    finalDateFlag=false;
    lifeTimeFlag=false;
    intValueFlag=false;
  }
 
  public String toString()
  {
    StringBuilder sb=new StringBuilder();
    sb.append("BC_INC:");
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
    if(!intValueFlag)
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
 
    writer.writeShort((short)10); // tag id
    writer.writeIntLV(intValue);
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
        default:
          logger.warn("unknown tagId: "+tag+" seqnum: "+seqNum+" msg: "+BC_INC.class.getName());
      }

    }
  }
}
