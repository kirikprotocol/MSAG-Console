package mobi.eyeline.mcaia.protocol;

import java.util.HashMap;
import mobi.eyeline.protogen.framework.*;
import mobi.eyeline.mcaia.protocol.*;

public enum TestServerProtocolTag {
    BusyRequest(1, BusyRequest.class),
    BusyResponse(2, BusyResponse.class);
    int value;
  Class pduClass;

  TestServerProtocolTag(int value, Class c) {
    this.value = value;
    this.pduClass = c;
  }

  public int getValue() {
    return value;
  }

  static HashMap<Integer, TestServerProtocolTag> map;
  static {
    map = new HashMap<Integer, TestServerProtocolTag>();
    for( TestServerProtocolTag t : TestServerProtocolTag.values() ) {
      map.put( t.getValue(), t );
    }
  }

  public static TestServerProtocolTag valueOf( int v ) {
    return map.get(v);
  }

  public PDU createPdu() {
    try {
      return (PDU)pduClass.newInstance();
    } catch (Exception e) {
      throw new IllegalArgumentException("Could not create PDU class for "+name(), e);
    }
  }
}
