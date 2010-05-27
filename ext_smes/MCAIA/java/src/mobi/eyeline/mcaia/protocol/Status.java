package mobi.eyeline.mcaia.protocol;

import mobi.eyeline.protogen.framework.*;
import java.util.HashMap;




public enum Status {
    OK((byte)0),
    InvalidCalledAddress((byte)1),
    ExecutingError((byte)2),
    Timedout((byte)3);
  
  byte value;
  public byte getValue() {
    return value;
  }

  Status( byte value ) {
    this.value = value;
  }
  static Status[] map;
  static {
    int maxVal = 0;
    for( Status i : Status.values() ) {
      if( maxVal < ((int)i.value&0xFF) ) maxVal = ((int)i.value)&0xFF;
    }
    map = new Status[maxVal+1];
    for( Status i : Status.values() ) {
      map[((int)i.value)&0xFF] = i;
    }
  }

  public static Status valueOf( byte v ) throws InvalidEnumValueException {
    if( (((int)v)&0xFF) >= map.length ) throw new InvalidEnumValueException("Value "+v+" out of range "+map.length+" for enum Status");
    Status e = map[((int)v)&0xFF];
    if( e == null ) throw new InvalidEnumValueException("Value "+v+" invalid for enum Status");
    return e;
  }
}