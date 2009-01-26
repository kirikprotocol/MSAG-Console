package pvss.pvap;

import protogen.framework.BufferWriter;
import protogen.framework.BufferReader;

import java.io.IOException;

import org.apache.log4j.Logger;


public class PC_AUTH_RESP {
  static Logger logger = Logger.getLogger(PC_AUTH_RESP.class);

  int seqNum;
  byte status;
  boolean statusFlag=false;
  byte clientType;
  boolean clientTypeFlag=false;
  byte sid;
  boolean sidFlag=false;

  public PC_AUTH_RESP() {
  }

  public PC_AUTH_RESP(int seqNum,byte status, byte clientType, byte sid)
  {
    this.seqNum = seqNum;
    this.status = status;
    this.statusFlag = true;
    this.clientType = clientType;
    this.clientTypeFlag = true;
    this.sid = sid;
    this.sidFlag = true;
  }
 
  public void clear()
  {
    statusFlag=false;
    clientTypeFlag=false;
    sidFlag=false;
  }
 
  public String toString()
  {
    StringBuilder sb=new StringBuilder();
    sb.append("PC_AUTH_RESP:");
    sb.append("seqNum=");
    sb.append(seqNum);
    if(statusFlag)
    {
      sb.append(";status=");
      sb.append(status.toString());    }
    if(clientTypeFlag)
    {
      sb.append(";clientType=");
      sb.append(clientType.toString());    }
    if(sidFlag)
    {
      sb.append(";sid=");
      sb.append(sid.toString());    }
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
  public byte getClientType()
  {
    if(!clientTypeFlag)
    {
      //!!TODO!!
    }
    return clientType;
  }

  public void setClientType(byte clientType)
  {
    this.clientType = clientType;
    this.clientTypeFlag = true;
  }
 
  public boolean hasClientType()
  {
    return clientTypeFlag;
  }
  public byte getSid()
  {
    if(!sidFlag)
    {
      //!!TODO!!
    }
    return sid;
  }

  public void setSid(byte sid)
  {
    this.sid = sid;
    this.sidFlag = true;
  }
 
  public boolean hasSid()
  {
    return sidFlag;
  }

  public void encode(BufferWriter writer)
  {
    if(!statusFlag)
    {
      //!!TODO!!
    }
    writer.writeInt( seqNum );
 
    writer.writeShort((short)1); // tag id
    writer.writeByteLV(status);
    if(clientTypeFlag)
    { 
      writer.writeShort((short)19); // tag id
      writer.writeByteLV(clientType);
    }
    if(sidFlag)
    { 
      writer.writeShort((short)20); // tag id
      writer.writeByteLV(sid);
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
        case 1:{
          status = reader.readByteLV();          statusFlag=true;
          }break;
        case 19:{
          clientType = reader.readByteLV();          clientTypeFlag=true;
          }break;
        case 20:{
          sid = reader.readByteLV();          sidFlag=true;
          }break;
        default:
          logger.warn("unknown tagId: "+tag+" seqnum: "+seqNum+" msg: "+PC_AUTH_RESP.class.getName());
      }

    }
  }
}
