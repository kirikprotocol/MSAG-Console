package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigSection;

/**
 * @author Aleksandr Khalitov
 */
public class Smsc {

  private final ValidationHelper vh = new ValidationHelper(Smsc.class);  //todo узнать ограничения валидатора !!

  private String name;

  private String host;

  private int port;

  private String systemId;

  private String password;

  private String systemType;

  private int interfaceVersion;

  private int ussdServiceOp;

  private int vlrUssdServiceOp;

  private Smsc() {
  }

  public Smsc(String name) throws AdminException{
    vh.checkNotEmpty("name", name);
    this.name = name;
  }

  void load(XmlConfigSection s) throws XmlConfigException {
    host = s.getString("host");
    port = s.getInt("port");
    interfaceVersion = s.getInt("interfaceVersion");
    password = s.getString("password");
    systemId = s.getString("sid");
    systemType = s.getString("systemType");
    ussdServiceOp = s.getInt("ussdPushTag");
    vlrUssdServiceOp = s.getInt("ussdPushVlrTag");
  }

  void save(XmlConfigSection s) throws XmlConfigException {

    s.setString("host", host);
    s.setInt("port", port);
    s.setInt("interfaceVersion", interfaceVersion);
    s.setString("password", password);
    s.setString("sid", systemId);
    s.setString("systemType", systemType);
    s.setInt("ussdPushTag", ussdServiceOp);
    s.setInt("ussdPushVlrTag", vlrUssdServiceOp);
  }

  public String getName() {
    return name;
  }

  public void setName(String name) throws AdminException{
    vh.checkNotEmpty("name", name);
    this.name = name;
  }

  public String getHost() {
    return host;
  }

  public void setHost(String host) throws AdminException{
    vh.checkNotEmpty("host", host);
    this.host = host;
  }

  public int getPort() {
    return port;
  }

  public void setPort(int port) throws AdminException {
    vh.checkPort("port", port);
    this.port = port;
  }

  public String getSystemId()  {
    return systemId;
  }

  public void setSystemId(String systemId) throws AdminException{
    vh.checkNotEmpty("systemId", systemId);
    this.systemId = systemId;
  }

  public String getPassword() {
    return password;
  }

  public void setPassword(String password) throws AdminException{
    vh.checkNotNull("password", password);
    this.password = password;
  }

  public String getSystemType() {
    return systemType;
  }

  public void setSystemType(String systemType) throws AdminException{
    vh.checkNotEmpty("systemType", systemType);
    this.systemType = systemType;
  }

  public int getInterfaceVersion() {
    return interfaceVersion;
  }

  public void setInterfaceVersion(int interfaceVersion) throws AdminException{
    vh.checkGreaterOrEqualsTo("interfaceVersion", interfaceVersion, 0);
    this.interfaceVersion = interfaceVersion;
  }

  public int getUssdServiceOp() {
    return ussdServiceOp;
  }

  public void setUssdServiceOp(int ussdServiceOp) throws AdminException{
    vh.checkGreaterOrEqualsTo("ussdServiceOp", ussdServiceOp, 0);
    this.ussdServiceOp = ussdServiceOp;
  }

  public int getVlrUssdServiceOp() {
    return vlrUssdServiceOp;
  }

  public void setVlrUssdServiceOp(int vlrUssdServiceOp) throws AdminException{
    vh.checkGreaterOrEqualsTo("vlrUssdServiceOp", vlrUssdServiceOp, 0);
    this.vlrUssdServiceOp = vlrUssdServiceOp;
  }

  @SuppressWarnings({"RedundantIfStatement"})
  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Smsc smsc = (Smsc) o;

    if (interfaceVersion != smsc.interfaceVersion) return false;
    if (port != smsc.port) return false;
    if (ussdServiceOp != smsc.ussdServiceOp) return false;
    if (vlrUssdServiceOp != smsc.vlrUssdServiceOp) return false;
    if (host != null ? !host.equals(smsc.host) : smsc.host != null) return false;
    if (name != null ? !name.equals(smsc.name) : smsc.name != null) return false;
    if (password != null ? !password.equals(smsc.password) : smsc.password != null) return false;
    if (systemId != null ? !systemId.equals(smsc.systemId) : smsc.systemId != null) return false;
    if (systemType != null ? !systemType.equals(smsc.systemType) : smsc.systemType != null) return false;

    return true;
  }

  public Smsc cloneSmsc() {
    Smsc s = new Smsc();
    s.name = name;
    s.host=host;
    s.port=port;
    s.systemId=systemId;
    s.password=password;
    s.systemType=systemType;
    s.interfaceVersion=interfaceVersion;
    s.ussdServiceOp=ussdServiceOp;
    s.vlrUssdServiceOp=vlrUssdServiceOp;
    return s;
  }

}
