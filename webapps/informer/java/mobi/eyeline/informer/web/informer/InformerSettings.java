package mobi.eyeline.informer.web.informer;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigSection;
import mobi.eyeline.informer.web.util.ValidationHelper;

/**
 * @author Aleksandr Khalitov
 */
public class InformerSettings {

  private final ValidationHelper vh = new ValidationHelper(InformerSettings.class);

  private String host;

  private int adminPort;

  private int deliveriesPort;

  void load(XmlConfig config) throws XmlConfigException{
    if(config.containsSection("informer")){
      XmlConfigSection s = config.getSection("informer");
      host = s.getString("host", null);
      adminPort = s.getInt("adminPort", 0);
      deliveriesPort = s.getInt("deliveriesPort", 0);
    }
  }

  void save(XmlConfig config) throws XmlConfigException{
    XmlConfigSection s = config.getOrCreateSection("informer");
    s.setString("host", host);
    s.setInt("adminPort", adminPort);
    s.setInt("deliveriesPort", deliveriesPort);
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

  public InformerSettings cloneSettings(){
    InformerSettings cs = new InformerSettings();
    cs.adminPort = adminPort;
    cs.deliveriesPort = deliveriesPort;
    cs.host = host;
    return cs;
  }
}
