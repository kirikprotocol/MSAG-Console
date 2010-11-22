package mobi.eyeline.informer.web.controllers.smsc;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.smsc.SmscException;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableRow;
import mobi.eyeline.informer.web.config.Configuration;
import org.apache.log4j.Logger;

import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * @author Aleksandr Khalitov
 */
public class SmscEditController extends SmscController {

  private static final Logger logger = Logger.getLogger(SmscEditController.class);

  private String oldName;

  private final SmscInfo smsc = new SmscInfo();

  private DynamicTableModel tempErrors = new DynamicTableModel();

  public SmscEditController() {
    super();

    oldName = getRequestParameter(SMSC_PARAMETER);

    if (oldName != null && oldName.length() > 0) {
      try {
        reload();
      } catch (AdminException e) {
        addError(e);
      }
    } else {
      smsc.setVlrUssdServiceOp(153);
      smsc.setUssdServiceOp(131);
      smsc.setInterfaceVersion(34);
      smsc.setMaxValidityPeriod(10);
      smsc.setMinValidityPeriod(1);
      smsc.setDefaultValidityPeriod(3);
      smsc.setInterConnectTimeout(60);
      smsc.setTimeout(10);
    }
  }

  private void reload() throws AdminException {
    Smsc s = getConfig().getSmsc(oldName);
    if (s != null) {
      convert(smsc, s);
      tempErrors = new DynamicTableModel();
      for (Map.Entry<String, Collection<Integer>> e : s.getTemporaryErrors().entrySet()) {
        DynamicTableRow row = new DynamicTableRow();
        row.setValue("interval", e.getKey());
        row.setValue("errors", convertToCSV(e.getValue()));
        tempErrors.addRow(row);
      }
    } else {
      logger.warn("SMSC is not found with name=" + oldName);
      oldName = null;
    }
  }

  private static String convertToCSV(Collection<Integer> c) {
    StringBuilder errors = new StringBuilder();
    for (Integer i : c) {
      errors.append(',').append(i);
    }
    return errors.length() > 0 ? errors.substring(1) : "";
  }

  private static Set<Integer> parseFromCSV(String s) throws AdminException {
    String[] cs = s.split(",");
    Set<Integer> result = new HashSet<Integer>(cs.length);
    for (String c : cs) {
      try {
        result.add(Integer.parseInt(c.trim()));
      } catch (NumberFormatException e) {
        throw new SmscException("illegal_code", c);
      }
    }
    return result;
  }

  private static Smsc convert(SmscInfo smsc, Smsc s) {
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
    smsc.immedErrors = convertToCSV(s.getImmediateErrors());
    smsc.permErrors = convertToCSV(s.getPermanentErrors());
    smsc.defaultValidityPeriod = s.getDefaultValidityPeriod();
    smsc.maxValidityPeriod = s.getMaxValidityPeriod();
    smsc.minValidityPeriod = s.getMinValidityPeriod();
    smsc.interConnectTimeout = s.getInterConnectTimeout();
    return s;
  }


  private static Smsc convert(SmscInfo smsc) throws AdminException {
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
    s.setDefaultValidityPeriod(smsc.defaultValidityPeriod);
    s.setMaxValidityPeriod(smsc.maxValidityPeriod);
    s.setMinValidityPeriod(smsc.minValidityPeriod);
    s.setInterConnectTimeout(smsc.interConnectTimeout);
    if(smsc.immedErrors != null && (smsc.immedErrors = smsc.immedErrors.trim()).length() != 0) {
      for(Integer i : parseFromCSV(smsc.immedErrors)) {
        s.addImmediateError(i);
      }
    }
    if (smsc.permErrors != null && (smsc.permErrors = smsc.permErrors.trim()).length() != 0) {
      for (Integer i : parseFromCSV(smsc.permErrors)) {
        s.addPermanentError(i);
      }
    }
    return s;
  }


  public String save() {
    Configuration config = getConfig();
    String user = getUserName();
    try {
      config.lock();
      Smsc s = convert(smsc);

      for (DynamicTableRow r : tempErrors.getRows()) {
        String interval = (String) r.getValue("interval");
        String errors = (String) r.getValue("errors");
        if (interval == null || (interval = interval.trim()).length() == 0
            || !Smsc.RETRY_POLICY_PATTERN.matcher(interval).matches()) {
          throw new SmscException("illegal_intervals", interval);
        }
        if (errors == null || (errors = errors.trim()).length() == 0) {
          throw new SmscException("illegal_code", "");
        }
        s.addTempError(interval, parseFromCSV(errors));
      }

      if (oldName != null && oldName.length() > 0) {

        if (!oldName.equals(smsc.name)) {

          config.addSmsc(s, user);
          if (config.getDefaultSmsc().equals(oldName)) {
            config.setDefaultSmsc(smsc.name, user);
          }
          config.removeSmsc(oldName, user);
        } else {
          config.updateSmsc(s, user);
        }

      } else {
        config.addSmsc(s, user);
      }

      return "SMSC";
    } catch (AdminException e) {
      addError(e);
      return null;
    } finally {
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

  public DynamicTableModel getTempErrors() {
    return tempErrors;
  }

  public void setTempErrors(DynamicTableModel tempErrors) {
    this.tempErrors = tempErrors;
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

    private String immedErrors;

    private String permErrors;

    private int defaultValidityPeriod;

    private int minValidityPeriod;

    private int maxValidityPeriod;

    private int interConnectTimeout;

    public int getDefaultValidityPeriod() {
      return defaultValidityPeriod;
    }

    public void setDefaultValidityPeriod(int defaultValidityPeriod) {
      this.defaultValidityPeriod = defaultValidityPeriod;
    }

    public int getMinValidityPeriod() {
      return minValidityPeriod;
    }

    public void setMinValidityPeriod(int minValidityPeriod) {
      this.minValidityPeriod = minValidityPeriod;
    }

    public int getMaxValidityPeriod() {
      return maxValidityPeriod;
    }

    public void setMaxValidityPeriod(int maxValidityPeriod) {
      this.maxValidityPeriod = maxValidityPeriod;
    }

    public String getImmedErrors() {
      return immedErrors;
    }

    public void setImmedErrors(String immedErrors) {
      this.immedErrors = immedErrors;
    }

    public String getPermErrors() {
      return permErrors;
    }

    public void setPermErrors(String permErrors) {
      this.permErrors = permErrors;
    }

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

    public int getInterConnectTimeout() {
      return interConnectTimeout;
    }

    public void setInterConnectTimeout(int interConnectTimeout) {
      this.interConnectTimeout = interConnectTimeout;
    }
  }

}
