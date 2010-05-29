package mobi.eyeline.mcaia.protocol;

import mobi.eyeline.protogen.framework.*;
import java.io.IOException;



/**
 * This class was generated by protogen, do not edit it
 */
public class BusyResponse extends BusyRequest {
 
  Status status;
  boolean statusFlag=false;

  public BusyResponse() {
    super();
  }
 

  public BusyResponse(int seqNum , String caller, String called, long date, byte cause, byte flags , Status status) {
    super( seqNum, caller, called, date, cause, flags);
    this.status = status;
    this.statusFlag = true;
  }
 
  public int getTag() {
    return 2;
  }

  public void clear() {
    super.clear();
    statusFlag=false;
  }
 
  public void debug(StringBuilder sb) {
    super.debug(sb);
    sb.append(';');

    if(statusFlag) { sb.append("status=") .append(status); }
  }
 
  public String toString() {
    StringBuilder sb=new StringBuilder(128);
    sb.append("BusyResponse:");
    debug(sb);
    return sb.toString();
  }


  public Status getStatus() throws AccessToUndefinedFieldException {
    if(!statusFlag) throw new AccessToUndefinedFieldException("status");
    return status;
  }

  public void setStatus(Status status) {
    this.status = status;
    this.statusFlag = true;
  }
 
  public boolean hasStatus() {
    return statusFlag;
  }

  public void encode(BufferWriter writer) throws IOException {
    super.encode(writer);
    if(statusFlag) {
      writer.writeShort((short)4); // tag id
      writer.writeByteLV(status.getValue());
    }  else throw new MissingMandatoryFieldException("status");
    writer.writeShort((short)0xFFFF); // end message tag
  }

  public void decode(BufferReader reader) throws IOException {
    super.decode(reader);
    while( true ) {
      int tag = reader.readShort();
      if( tag == (short)0xFFFF ) break;
      switch( tag ) {
        case 4: {
          status = Status.valueOf(reader.readByteLV());
          statusFlag=true;
        } break;
        default:
          throw new IOException( "unknown tagId: "+tag +" msg: "+BusyResponse.class.getName());      }
    }
  }
}