package pvss.pvap;

import protogen.framework.BufferWriter;
import protogen.framework.BufferReader;

import java.io.IOException;

import org.apache.log4j.Logger;


public class PC_BATCH_RESP {
  static Logger logger = Logger.getLogger(PC_BATCH_RESP.class);

  int seqNum;
  byte status;
  boolean statusFlag=false;
  short count;
  boolean countFlag=false;

  public PC_BATCH_RESP() {
  }

  public PC_BATCH_RESP(int seqNum,byte status, short count)
  {
    this.seqNum = seqNum;
    this.status = status;
    this.statusFlag = true;
    this.count = count;
    this.countFlag = true;
  }
 
  public void clear()
  {
    statusFlag=false;
    countFlag=false;
  }
 
  public String toString()
  {
    StringBuilder sb=new StringBuilder();
    sb.append("PC_BATCH_RESP:");
    sb.append("seqNum=");
    sb.append(seqNum);
    if(statusFlag)
    {
      sb.append(";status=");
      sb.append(status.toString());    }
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
    if(!statusFlag)
    {
      //!!TODO!!
    }
    if(!countFlag)
    {
      //!!TODO!!
    }
    writer.writeInt( seqNum );
 
    writer.writeShort((short)1); // tag id
    writer.writeByteLV(status);
 
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
        case 1:{
          status = reader.readByteLV();          statusFlag=true;
          }break;
        case 22:{
          count = reader.readShortLV();          countFlag=true;
          }break;
        default:
          logger.warn("unknown tagId: "+tag+" seqnum: "+seqNum+" msg: "+PC_BATCH_RESP.class.getName());
      }

    }
  }
}
