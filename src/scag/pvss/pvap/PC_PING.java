package pvss.pvap;

import protogen.framework.BufferWriter;
import protogen.framework.BufferReader;

import java.io.IOException;

import org.apache.log4j.Logger;


public class PC_PING {
  static Logger logger = Logger.getLogger(PC_PING.class);

  int seqNum;

  public PC_PING() {
  }

  public PC_PING(int seqNum,)
  {
    this.seqNum = seqNum;
  }
 
  public void clear()
  {
  }
 
  public String toString()
  {
    StringBuilder sb=new StringBuilder();
    sb.append("PC_PING:");
    sb.append("seqNum=");
    sb.append(seqNum);
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


  public void encode(BufferWriter writer)
  {
    writer.writeInt( seqNum );
    writer.writeShort((short)0xFFFF); // end message tag
  }

  public void decode(BufferReader reader) throws IOException
  {
    seqNum = reader.readInt();
    while( true ) {
      int tag = reader.readShort();
      if( tag == (short)0xFFFF ) break;
      switch( tag ) {
        default:
          logger.warn("unknown tagId: "+tag+" seqnum: "+seqNum+" msg: "+PC_PING.class.getName());
      }

    }
  }
}
