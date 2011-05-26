package mobi.eyeline.informer.admin.cdr;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.config.XmlConfigParam;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.util.Properties;

/**
 * @author Aleksandr Khalitov
 */
public class CdrSettings {

  private final  ValidationHelper vh = new ValidationHelper(this.getClass());

  private final Properties properties = new Properties();

  public CdrSettings() {
  }

  public CdrSettings(CdrSettings s) {
    properties.putAll(s.getAllProperties());
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

  public void load(XmlConfigSection section) {
    this.properties.clear();
    this.properties.putAll(section.toProperties("", null));
  }

  public void save(XmlConfigSection section) {
    for (Object s : properties.keySet()) {
      section.addParam(new XmlConfigParam((String) s, (String) properties.get(s), XmlConfigParam.Type.STRING));
    }
  }
}
