package mobi.eyeline.informer.web.controllers.smsc;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.web.config.Configuration;
import org.apache.log4j.Logger;

/**
 * @author Aleksandr Khalitov
 */
public class SmscEditController extends SmscController{

  private static final Logger logger = Logger.getLogger(SmscEditController.class);

  private String oldName;

  private SmscInfo smsc = new SmscInfo();

  public SmscEditController() {
    super();

    oldName = getRequestParameter(SMSC_PARAMETER);

    if(oldName != null && oldName.length() > 0) {
      try{
        reload();
      }catch (AdminException e){
        addError(e);
      }
    }
  }

  private void reload() throws AdminException{
    Smsc s = getConfig().getSmsc(oldName);
    if(s != null) {
      convert(smsc, s);
    }else {
      logger.warn("SMSC is not found with name="+oldName);
      oldName = null;
    }
  }

  private static Smsc convert(SmscInfo smsc, Smsc s) throws AdminException{
    smsc.name = s.getName();
    smsc.host = s.getHost();
    smsc.name = s.getName();
    smsc.port = s.getPort();
    smsc.systemId = s.getSystemId();
    smsc.password = s.getPassword();
    smsc.systemType = s.getSystemType();
    smsc.interfaceVersion = s.getInterfaceVersion();
    smsc.ussdServiceOp = s.getUssdServiceOp();
    smsc.vlrUssdServiceOp = s.getVlrUssdServiceOp();
    smsc.timeout = s.getTimeout();
    smsc.rangeOfAddress = s.getRangeOfAddress();
    return s;
  }


  private static Smsc convert(SmscInfo smsc) throws AdminException{
    Smsc s = new Smsc(smsc.name);
    s.setHost(smsc.host);
    s.setInterfaceVersion(smsc.interfaceVersion);
    s.setPassword(smsc.password);
    s.setPort(smsc.port);
    s.setSystemId(smsc.systemId);
    s.setSystemType(smsc.systemType);
    s.setUssdServiceOp(smsc.ussdServiceOp);
    s.setVlrUssdServiceOp(smsc.vlrUssdServiceOp);
    s.setTimeout(smsc.timeout);
    s.setRangeOfAddress(smsc.rangeOfAddress);
    return s;
  }


  public String save() {
    Configuration config = getConfig();
    String user = getUserName();
    try{
      config.lock();
      Smsc s = convert(smsc);

      if(oldName != null && oldName.length() > 0) {

        if(!oldName.equals(smsc.name)) {
          
          config.addSmsc(s, user);
          if(config.getDefaultSmsc().equals(oldName)) {
            config.setDefaultSmsc(smsc.name, user);
          }
          config.removeSmsc(oldName, user);
        }else {
          config.updateSmsc(s, user);
        }

      }else {
        config.addSmsc(s, user);
      }

      return "SMSC";
    }catch (AdminException e){
      addError(e);
      return null;
    }finally {
      config.unlock();
    }

  }

  public String getOldName() {
    return oldName;
  }

  public void setOldName(String oldName) {
    this.oldName = oldName;
  }

  public SmscInfo getSmsc() {
    return smsc;
  }

  public static class SmscInfo {

    private String name;

    private String host;

    private int port;

    private String systemId;

    private String password;

    private String systemType;

    private int interfaceVersion;

    private int ussdServiceOp;

    private int vlrUssdServiceOp;

    private int timeout;

    private int rangeOfAddress;

    public int getTimeout() {
      return timeout;
    }

    public void setTimeout(int timeout) {
      this.timeout = timeout;
    }

    public int getRangeOfAddress() {
      return rangeOfAddress;
    }

    public void setRangeOfAddress(int rangeOfAddress) {
      this.rangeOfAddress = rangeOfAddress;
    }

    public String getName() {
      return name;
    }

    public void setName(String name) {
      this.name = name;
    }

    public String getHost() {
      return host;
    }

    public void setHost(String host) {
      this.host = host;
    }

    public int getPort() {
      return port;
    }

    public void setPort(int port) {
      this.port = port;
    }

    public String getSystemId() {
      return systemId;
    }

    public void setSystemId(String systemId) {
      this.systemId = systemId;
    }

    public String getPassword() {
      return password;
    }

    public void setPassword(String password) {
      this.password = password;
    }

    public String getSystemType() {
      return systemType;
    }

    public void setSystemType(String systemType) {
      this.systemType = systemType;
    }

    public int getInterfaceVersion() {
      return interfaceVersion;
    }

    public void setInterfaceVersion(int interfaceVersion) {
      this.interfaceVersion = interfaceVersion;
    }

    public int getUssdServiceOp() {
      return ussdServiceOp;
    }

    public void setUssdServiceOp(int ussdServiceOp) {
      this.ussdServiceOp = ussdServiceOp;
    }

    public int getVlrUssdServiceOp() {
      return vlrUssdServiceOp;
    }

    public void setVlrUssdServiceOp(int vlrUssdServiceOp) {
      this.vlrUssdServiceOp = vlrUssdServiceOp;
    }
  }

}
