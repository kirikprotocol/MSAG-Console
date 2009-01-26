package protogen;

import protogen.framework.BufferReader;
import protogen.framework.InvalidMessageTypeException;
import protogen.framework.BufferWriter;

import java.io.IOException;


/**
 * This protocol has header: len (4 oct), tag (4 oct), seq (4 oct), message body
 *
 *
 */
public class PVAP {

  public static interface Handler {
    public void handle( PC_DEL object );
    public void handle( PC_DEL_RESP object );
    public void handle( PC_SET object );
    public void handle( PC_SET_RESP object );
    public void handle( PC_GET object );
    public void handle( PC_GET_RESP object );
    public void handle( PC_INC object );
    public void handle( PC_INC_RESP object );
    public void handle( PC_INC_MOD object );
    public void handle( PC_INC_MOD_RESP object );
    public void handle( PC_PING object );
    public void handle( PC_PING_RESP object );
    public void handle( PC_AUTH object );
    public void handle( PC_AUTH_RESP object );
    public void handle( PC_BATCH object );
    public void handle( PC_BATCH_RESP object );
  }


  Handler handler;
  public PVAP(Handler handler) {
    this.handler = handler;
  }

  public void decodeMessage( BufferReader reader ) throws IOException {
    int tag = reader.readInt();
    switch( tag ) {
      case 1:
      {
        PC_DEL object = new PC_DEL();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32769:
      {
        PC_DEL_RESP object = new PC_DEL_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 2:
      {
        PC_SET object = new PC_SET();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32770:
      {
        PC_SET_RESP object = new PC_SET_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 3:
      {
        PC_GET object = new PC_GET();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32771:
      {
        PC_GET_RESP object = new PC_GET_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 4:
      {
        PC_INC object = new PC_INC();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32772:
      {
        PC_INC_RESP object = new PC_INC_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 5:
      {
        PC_INC_MOD object = new PC_INC_MOD();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32773:
      {
        PC_INC_MOD_RESP object = new PC_INC_MOD_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 6:
      {
        PC_PING object = new PC_PING();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32774:
      {
        PC_PING_RESP object = new PC_PING_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 10:
      {
        PC_AUTH object = new PC_AUTH();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32778:
      {
        PC_AUTH_RESP object = new PC_AUTH_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 7:
      {
        PC_BATCH object = new PC_BATCH();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32775:
      {
        PC_BATCH_RESP object = new PC_BATCH_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      default: throw new InvalidMessageTypeException();
    }

  }
  public byte[] encodeMessage( PC_DEL object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(1);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_DEL_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32769);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_SET object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(2);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_SET_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32770);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_GET object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(3);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_GET_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32771);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_INC object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(4);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_INC_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32772);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_INC_MOD object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(5);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_INC_MOD_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32773);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_PING object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(6);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_PING_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32774);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_AUTH object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(10);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_AUTH_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32778);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_BATCH object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(7);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( PC_BATCH_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32775);
    object.encode(writer);
    return writer.getData();
  }

}
