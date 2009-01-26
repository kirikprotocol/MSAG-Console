package pvss.pvap;

import protogen.framework.BufferWriter;
import protogen.framework.BufferReader;

import java.io.IOException;

import org.apache.log4j.Logger;


public class PC_INC_MOD_RESP {
  static Logger logger = Logger.getLogger(PC_INC_MOD_RESP.class);

  int seqNum;
  byte status;
  boolean statusFlag=false;
  int intValue;
  boolean intValueFlag=false;

  public PC_INC_MOD_RESP() {
  }

  public PC_INC_MOD_RESP(int seqNum,byte status, int intValue)
  {
    this.seqNum = seqNum;
    this.status = status;
    this.statusFlag = true;
    this.intValue = intValue;
    this.intValueFlag = true;
  }
 
  public void clear()
  {
    statusFlag=false;
    intValueFlag=false;
  }
 
  public String toString()
  {
    StringBuilder sb=new StringBuilder();
    sb.append("PC_INC_MOD_RESP:");
    sb.append("seqNum=");
    sb.append(seqNum);
    if(statusFlag)
    {
      sb.append(";status=");
      sb.append(status.toString());    }
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

  public byte getStatus()
  {
    if(!statusFlag)
    {
      //!!TODO!!
    }
    return status;
  }

  public void setStatus(byte status)
  {
    this.status = status;
    this.statusFlag = true;
  }
 
  public boolean hasStatus()
  {
    return statusFlag;
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
    if(!statusFlag)
    {
      //!!TODO!!
    }
    if(!intValueFlag)
    {
      //!!TODO!!
    }
    writer.writeInt( seqNum );
 
    writer.writeShort((short)1); // tag id
    writer.writeByteLV(status);
 
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
        case 1:{
          status = reader.readByteLV();          statusFlag=true;
          }break;
        case 10:{
          intValue = reader.readIntLV();          intValueFlag=true;
          }break;
        default:
          logger.warn("unknown tagId: "+tag+" seqnum: "+seqNum+" msg: "+PC_INC_MOD_RESP.class.getName());
      }

    }
  }
}
