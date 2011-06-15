package mobi.eyeline.informer.admin.delivery.protogen.protocol;

import mobi.eyeline.protogen.framework.*;
import java.io.IOException;



/**
 * This class was generated by protogen, do not edit it
 */
public class OkResponse implements PDU {
  int seqNum;
  Connection connection;
 

  public OkResponse() {
  }
 

  public OkResponse(int seqNum) {
 
    this.seqNum = seqNum;
  }
 
  public int getTag() {
    return 1;
  }

  public void clear() {
  }
 
  public void debug(StringBuilder sb) {
    sb.append("seqNum=").append(seqNum).append(';');
  }
 
  public String toString() {
    StringBuilder sb=new StringBuilder(128);
    sb.append("OkResponse:");
    debug(sb);
    return sb.toString();
  }

  public int getSeqNum() {
    return seqNum;
  }

  public void setSeqNum(int seqNum) {
    this.seqNum = seqNum;
  }
 
  public int assignSeqNum() {
    seqNum = PDU.seqGenerator.incrementAndGet();
    return seqNum;
  }
 
  public Connection getConnection() {
    return connection;
  }
 
  public void setConnection( Connection con ) {
    connection = con;
  }


  public void encode(BufferWriter writer) throws IOException {
    writer.writeTag(0xFF); // end message tag
  }

  public void decode(BufferReader reader) throws IOException {
    while( true ) {
      int tag = reader.readTag();
      if( tag == 0xFF ) break;
      switch( tag ) {
        default:
          throw new IOException( "unknown tagId: "+tag +" seqnum: "+seqNum +" msg: "+OkResponse.class.getName());      }
    }
  }
}