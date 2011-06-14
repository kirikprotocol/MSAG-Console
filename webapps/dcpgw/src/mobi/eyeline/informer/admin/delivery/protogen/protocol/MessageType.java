package mobi.eyeline.informer.admin.delivery.protogen.protocol;

import mobi.eyeline.protogen.framework.*;
import java.util.HashMap;




public enum MessageType {
    TextMessage((byte)1),
    GlossaryMessage((byte)2);
  
  byte value;
  public byte getValue() {
    return value;
  }

  MessageType( byte value ) {
    this.value = value;
  }
  static MessageType[] map;
  static {
    int maxVal = 0;
    for( MessageType i : MessageType.values() ) {
      if( maxVal < ((int)i.value&0xFF) ) maxVal = ((int)i.value)&0xFF;
    }
    map = new MessageType[maxVal+1];
    for( MessageType i : MessageType.values() ) {
      map[((int)i.value)&0xFF] = i;
    }
  }

  public static MessageType valueOf( byte v ) throws InvalidEnumValueException {
    if( (((int)v)&0xFF) >= map.length ) throw new InvalidEnumValueException("Value "+v+" out of range "+map.length+" for enum MessageType");
    MessageType e = map[((int)v)&0xFF];
    if( e == null ) throw new InvalidEnumValueException("Value "+v+" invalid for enum MessageType");
    return e;
  }
}