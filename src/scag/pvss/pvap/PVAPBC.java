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
public class PVAPBC {

  public static interface Handler {
    public void handle( BC_DEL object );
    public void handle( BC_DEL_RESP object );
    public void handle( BC_SET object );
    public void handle( BC_SET_RESP object );
    public void handle( BC_GET object );
    public void handle( BC_GET_RESP object );
    public void handle( BC_INC object );
    public void handle( BC_INC_RESP object );
    public void handle( BC_INC_MOD object );
    public void handle( BC_INC_MOD_RESP object );
  }


  Handler handler;
  public PVAPBC(Handler handler) {
    this.handler = handler;
  }

  public void decodeMessage( BufferReader reader ) throws IOException {
    int tag = reader.readInt();
    switch( tag ) {
      case 1:
      {
        BC_DEL object = new BC_DEL();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32769:
      {
        BC_DEL_RESP object = new BC_DEL_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 2:
      {
        BC_SET object = new BC_SET();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32770:
      {
        BC_SET_RESP object = new BC_SET_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 3:
      {
        BC_GET object = new BC_GET();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32771:
      {
        BC_GET_RESP object = new BC_GET_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 4:
      {
        BC_INC object = new BC_INC();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32772:
      {
        BC_INC_RESP object = new BC_INC_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 5:
      {
        BC_INC_MOD object = new BC_INC_MOD();
        object.decode(reader);
        handler.handle(object);
      }break;
      case 32773:
      {
        BC_INC_MOD_RESP object = new BC_INC_MOD_RESP();
        object.decode(reader);
        handler.handle(object);
      }break;
      default: throw new InvalidMessageTypeException();
    }

  }
  public byte[] encodeMessage( BC_DEL object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(1);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( BC_DEL_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32769);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( BC_SET object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(2);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( BC_SET_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32770);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( BC_GET object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(3);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( BC_GET_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32771);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( BC_INC object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(4);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( BC_INC_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32772);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( BC_INC_MOD object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(5);
    object.encode(writer);
    return writer.getData();
  }
  public byte[] encodeMessage( BC_INC_MOD_RESP object) {
    BufferWriter writer = new BufferWriter();
    writer.writeInt(32773);
    object.encode(writer);
    return writer.getData();
  }

}
