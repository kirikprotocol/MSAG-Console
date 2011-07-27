import java.util.HashMap;
import mobi.eyeline.protogen.framework.*;

public enum UpdateConfigClientTag {
    UpdateConfig(1, UpdateConfig.class),
    UpdateConfigResp(2, UpdateConfigResp.class);
    int value;
  Class pduClass;

  UpdateConfigClientTag(int value, Class c) {
    this.value = value;
    this.pduClass = c;
  }

  public int getValue() {
    return value;
  }

  static HashMap<Integer, UpdateConfigClientTag> map;
  static {
    map = new HashMap<Integer, UpdateConfigClientTag>();
    for( UpdateConfigClientTag t : UpdateConfigClientTag.values() ) {
      map.put( t.getValue(), t );
    }
  }

  public static UpdateConfigClientTag valueOf( int v ) {
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
