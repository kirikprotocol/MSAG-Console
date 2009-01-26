package pvss.pvap;

import protogen.framework.BufferWriter;
import protogen.framework.BufferReader;

import java.io.IOException;

import org.apache.log4j.Logger;


public class PC_PING_RESP {
  static Logger logger = Logger.getLogger(PC_PING_RESP.class);

  int seqNum;
  byte status;
  boolean statusFlag=false;

  public PC_PING_RESP() {
  }

  public PC_PING_RESP(int seqNum,byte status)
  {
    this.seqNum = seqNum;
    this.status = status;
    this.statusFlag = true;
  }
 
  public void clear()
  {
    statusFlag=false;
  }
 
  public String toString()
  {
    StringBuilder sb=new StringBuilder();
    sb.append("PC_PING_RESP:");
    sb.append("seqNum=");
    sb.append(seqNum);
    if(statusFlag)
    {
      sb.append(";status=");
      sb.append(status.toString());    }
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

  public void encode(BufferWriter writer)
  {
    if(!statusFlag)
    {
      //!!TODO!!
    }
    writer.writeInt( seqNum );
 
    writer.writeShort((short)1); // tag id
    writer.writeByteLV(status);
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
        default:
          logger.warn("unknown tagId: "+tag+" seqnum: "+seqNum+" msg: "+PC_PING_RESP.class.getName());
      }

    }
  }
}
