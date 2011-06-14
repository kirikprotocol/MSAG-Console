package mobi.eyeline.informer.admin.delivery.protogen.protocol;

import mobi.eyeline.protogen.framework.*;
import java.util.HashMap;




public enum DeliveryMode {
    SMS((byte)1),
    USSD_PUSH((byte)2),
    USSD_PUSH_VLR((byte)3);
  
  byte value;
  public byte getValue() {
    return value;
  }

  DeliveryMode( byte value ) {
    this.value = value;
  }
  static DeliveryMode[] map;
  static {
    int maxVal = 0;
    for( DeliveryMode i : DeliveryMode.values() ) {
      if( maxVal < ((int)i.value&0xFF) ) maxVal = ((int)i.value)&0xFF;
    }
    map = new DeliveryMode[maxVal+1];
    for( DeliveryMode i : DeliveryMode.values() ) {
      map[((int)i.value)&0xFF] = i;
    }
  }

  public static DeliveryMode valueOf( byte v ) throws InvalidEnumValueException {
    if( (((int)v)&0xFF) >= map.length ) throw new InvalidEnumValueException("Value "+v+" out of range "+map.length+" for enum DeliveryMode");
    DeliveryMode e = map[((int)v)&0xFF];
    if( e == null ) throw new InvalidEnumValueException("Value "+v+" invalid for enum DeliveryMode");
    return e;
  }
}