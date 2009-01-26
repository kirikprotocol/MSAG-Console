package pvss.pvap;

import protogen.framework.BufferWriter;
import protogen.framework.BufferReader;

import java.io.IOException;

import org.apache.log4j.Logger;


public class BC_GET {
  static Logger logger = Logger.getLogger(BC_GET.class);

  int seqNum;
  String varName;
  boolean varNameFlag=false;

  public BC_GET() {
  }

  public BC_GET(int seqNum,String varName)
  {
    this.seqNum = seqNum;
    this.varName = varName;
    this.varNameFlag = true;
  }
 
  public void clear()
  {
    varNameFlag=false;
  }
 
  public String toString()
  {
    StringBuilder sb=new StringBuilder();
    sb.append("BC_GET:");
    sb.append("seqNum=");
    sb.append(seqNum);
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
    if(!varNameFlag)
    {
      //!!TODO!!
    }
    writer.writeInt( seqNum );
 
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
        case 5:{
          varName = reader.readStringLV();          varNameFlag=true;
          }break;
        default:
          logger.warn("unknown tagId: "+tag+" seqnum: "+seqNum+" msg: "+BC_GET.class.getName());
      }

    }
  }
}
