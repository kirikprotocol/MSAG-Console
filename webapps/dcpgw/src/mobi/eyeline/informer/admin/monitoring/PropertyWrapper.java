package mobi.eyeline.informer.admin.monitoring;

/**
 * @author Aleksandr Khalitov
 */
public class PropertyWrapper {

  private final boolean readable;
  private final boolean writable;
  private final boolean is;
  private final String type;

  protected PropertyWrapper(String type, boolean readable, boolean writable, boolean is) {
    this.readable = readable;
    this.writable = writable;
    this.is = is;
    this.type = type;
  }

  public String getType() {
    return type;
  }

  public boolean isReadable() {
    return readable;
  }

  public boolean isWritable() {
    return writable;
  }

  public boolean isIs() {
    return is;
  }

  public Object getValue() {
    return null;
  }

  public void setValue(Object value) {
  }

}
