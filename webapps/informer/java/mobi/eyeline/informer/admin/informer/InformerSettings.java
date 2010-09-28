package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigSection;

/**
 * Настройки Informer
 * @author Aleksandr Khalitov
 */
public class InformerSettings {

  private final ValidationHelper vh = new ValidationHelper(InformerSettings.class);

  private String host;

  private int adminPort;

  private int deliveriesPort;

  private String persHost;

  private int persPort;

  void load(XmlConfig config) throws XmlConfigException{
    XmlConfigSection s = config.getSection("informer");
    host = s.getString("host", null);
    adminPort = s.getInt("adminPort", 0);
    deliveriesPort = s.getInt("deliveriesPort", 0);
    s = config.getSection("pvss");
    persHost = s.getString("host");
    persPort = s.getInt("port");
  }

  void save(XmlConfig config) throws XmlConfigException{
    XmlConfigSection s = config.getOrCreateSection("informer");
    s.setString("host", host);
    s.setInt("adminPort", adminPort);
    s.setInt("deliveriesPort", deliveriesPort);
    s = config.getOrCreateSection("pvss");
    s.setString("host", persHost);
    s.setInt("port", persPort);
  }

  public String getHost() {
    return host;
  }


  public void setHost(String host) throws AdminException{
    vh.checkNotEmpty("host", host);
    this.host = host;
  }

  public int getAdminPort() {
    return adminPort;
  }

  public void setAdminPort(int adminPort) throws AdminException {
    vh.checkPort("adminPort", adminPort);
    this.adminPort = adminPort;
  }

  public int getDeliveriesPort() {
    return deliveriesPort;
  }

  public void setDeliveriesPort(int deliveriesPort) throws AdminException {
    vh.checkPort("deliveriesPort", deliveriesPort);
    this.deliveriesPort = deliveriesPort;
  }

  public String getPersHost() {
    return persHost;
  }

  public void setPersHost(String persHost) throws AdminException{
    vh.checkNotEmpty("persHost", persHost);
    this.persHost = persHost;
  }

  public int getPersPort() {
    return persPort;
  }

  public void setPersPort(int persPort) throws AdminException{
    vh.checkPort("persPort", persPort);
    this.persPort = persPort;
  }

  /**
   * Копирует настройки
   * @return копия настроек
   */
  public InformerSettings cloneSettings(){
    InformerSettings cs = new InformerSettings();
    cs.adminPort = adminPort;
    cs.deliveriesPort = deliveriesPort;
    cs.host = host;
    cs.persHost = persHost;
    cs.persPort = persPort;
    return cs;
  }
}
