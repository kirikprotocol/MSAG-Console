package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

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
}
