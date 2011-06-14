package mobi.eyeline.informer.admin.delivery.protogen.protocol;

import mobi.eyeline.protogen.framework.*;
import java.util.HashMap;




public enum DeliveryStatus {
    Planned((byte)1),
    Active((byte)2),
    Paused((byte)3),
    Cancelled((byte)4),
    Finished((byte)5);
  
  byte value;
  public byte getValue() {
    return value;
  }

  DeliveryStatus( byte value ) {
    this.value = value;
  }
  static DeliveryStatus[] map;
  static {
    int maxVal = 0;
    for( DeliveryStatus i : DeliveryStatus.values() ) {
      if( maxVal < ((int)i.value&0xFF) ) maxVal = ((int)i.value)&0xFF;
    }
    map = new DeliveryStatus[maxVal+1];
    for( DeliveryStatus i : DeliveryStatus.values() ) {
      map[((int)i.value)&0xFF] = i;
    }
  }

  public static DeliveryStatus valueOf( byte v ) throws InvalidEnumValueException {
    if( (((int)v)&0xFF) >= map.length ) throw new InvalidEnumValueException("Value "+v+" out of range "+map.length+" for enum DeliveryStatus");
    DeliveryStatus e = map[((int)v)&0xFF];
    if( e == null ) throw new InvalidEnumValueException("Value "+v+" invalid for enum DeliveryStatus");
    return e;
  }
}