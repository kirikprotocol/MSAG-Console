package mobi.eyeline.informer.admin.cdr;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

import java.util.Properties;

/**
 * @author Aleksandr Khalitov
 */
public class CdrSettings {

  private final  ValidationHelper vh = new ValidationHelper(this.getClass());

  private final Properties properties;

  public CdrSettings(Properties properties) { // todo надо убрать этот конструктор
    this.properties = properties;
  }

  public CdrSettings(CdrSettings s) {
    this(s.getAllProperties());
  }

  void validate() throws AdminException {
    vh.checkNotEmpty("dir", getCdrDir());
  }

  public Properties getAllProperties() {
    Properties p = new Properties();
    p.putAll(properties);
    return p;
  }

  public String getCdrDir() {
    return properties.getProperty("dir");
  }

  public void setCdrDir(String cdrDir) {
    if(cdrDir != null) {
      properties.setProperty("dir", cdrDir);
    }else {
      properties.remove("dir");
    }
  }
}
