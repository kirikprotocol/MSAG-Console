package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigParam;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.util.*;
import java.util.regex.Pattern;

/**
 * @author Aleksandr Khalitov
 */
public class Smsc {

  private static final String LAST = "(\\d\\d{0,2}(m|h|s|d)(|:\\d{1,4}|:\\*))";
  private static final String MEDIUM = "(\\d\\d{0,2}(m|h|s|d)(|:\\d{1,4}))";
  public static final Pattern RETRY_POLICY_PATTERN = Pattern.compile("(" + LAST + "|" + MEDIUM + "(," + MEDIUM + ")*" + "(," + LAST + ")?" + ")");

  private final ValidationHelper vh = new ValidationHelper(Smsc.class);

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

  private Collection<Integer> immediateErrors = new LinkedList<Integer>();

  private Collection<Integer> permanentErrors = new LinkedList<Integer>();

  private Map<String, Collection<Integer>> temporaryErrors = new HashMap<String, Collection<Integer>>();

  private int defaultValidityPeriod;

  private int minValidityPeriod;

  private int maxValidityPeriod;

  private Smsc() {
  }

  public Smsc(String name) throws AdminException {
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
    timeout = s.getInt("timeout");
    rangeOfAddress = s.getInt("rangeOfAddress");
    defaultValidityPeriod = s.getInt("defaultValidityPeriod");
    maxValidityPeriod = s.getInt("maxValidityPeriod");
    minValidityPeriod = s.getInt("minValidityPeriod");
    s = s.getSection("retryPolicies");
    for (String e : s.getStringList("immediate", ",")) {
      immediateErrors.add(Integer.parseInt(e.trim()));
    }
    for (String e : s.getStringList("permanent", ",")) {
      permanentErrors.add(Integer.parseInt(e.trim()));
    }
    s = s.getSection("temporary");
    for (XmlConfigParam p : s.params()) {
      String interval = p.getName();
      Set<Integer> es = new HashSet<Integer>();
      for (String e : p.getStringList(",")) {
        es.add(Integer.parseInt(e.trim()));
      }
      temporaryErrors.put(interval, es);
    }
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
    s.setInt("timeout", timeout);
    s.setInt("rangeOfAddress", rangeOfAddress);
    s.setInt("defaultValidityPeriod", defaultValidityPeriod);
    s.setInt("maxValidityPeriod", maxValidityPeriod);
    s.setInt("minValidityPeriod", minValidityPeriod);
    s = s.getOrCreateSection("retryPolicies");
    s.setStringList("immediate", immediateErrors, ",");
    s.setStringList("permanent", permanentErrors, ",");
    s = s.getOrCreateSection("temporary");
    s.clear();
    for (Map.Entry<String, Collection<Integer>> e : temporaryErrors.entrySet()) {
      s.setStringList(e.getKey(), e.getValue(), ",");
    }
  }

  public int getMaxValidityPeriod() {
    return maxValidityPeriod;
  }

  public void setMaxValidityPeriod(int maxValidityPeriod) throws AdminException {
    vh.checkGreaterOrEqualsTo("maxValidityPeriod", maxValidityPeriod, 0);
    this.maxValidityPeriod = maxValidityPeriod;
  }

  public int getDefaultValidityPeriod() {
    return defaultValidityPeriod;
  }

  public void setDefaultValidityPeriod(int defaultValidityPeriod) throws AdminException {
    vh.checkGreaterOrEqualsTo("defaultValidityPeriod", defaultValidityPeriod, 0);
    this.defaultValidityPeriod = defaultValidityPeriod;
  }

  public int getMinValidityPeriod() {
    return minValidityPeriod;
  }

  public void setMinValidityPeriod(int minValidityPeriod) throws AdminException {
    vh.checkGreaterOrEqualsTo("minValidityPeriod", minValidityPeriod, 0);
    this.minValidityPeriod = minValidityPeriod;
  }

  public Collection<Integer> getImmediateErrors() {
    return new ArrayList<Integer>(immediateErrors);
  }

  public Collection<Integer> getPermanentErrors() {
    return new ArrayList<Integer>(permanentErrors);
  }

  public Map<String, Collection<Integer>> getTemporaryErrors() {
    Map<String, Collection<Integer>> result = new HashMap<String, Collection<Integer>>(temporaryErrors.size());

    for (Map.Entry<String, Collection<Integer>> e : temporaryErrors.entrySet()) {
      result.put(e.getKey(), new HashSet<Integer>(e.getValue()));
    }

    return result;
  }

  public void addImmediateError(int error) throws AdminException {
    vh.checkNotContains("immediateErrors", immediateErrors, error);
    immediateErrors.add(error);
  }

  public void removeImmediateError(int error) {
    immediateErrors.remove(error);
  }

  public void clearImmediateErrors() {
    immediateErrors.clear();
  }

  public void addPermanentError(int error) throws AdminException {
    vh.checkNotContains("permanentErrors", permanentErrors, error);
    permanentErrors.add(error);
  }

  public void clearPermanentErrors() {
    permanentErrors.clear();
  }

  public void removePermanentError(int error) {
    permanentErrors.remove(error);
  }

  public void addTempError(String period, Set<Integer> errors) throws AdminException {
    vh.checkNotEmpty("temporaryErrors", period);
    vh.checkMaches("temporaryErrors", period, RETRY_POLICY_PATTERN);
    vh.checkNotContains("temporaryErrors", temporaryErrors.keySet(), period);
    vh.checkSizeGreaterThan("temporaryErrors", errors, 0);
    temporaryErrors.put(period, errors);
  }

  public void clearTempErrors() {
    temporaryErrors.clear();
  }

  public void removeTempError(String period) {
    temporaryErrors.remove(period);
  }


  public int getTimeout() {
    return timeout;
  }

  public void setTimeout(int timeout) throws AdminException {
    vh.checkGreaterThan("timeout", timeout, 0);
    this.timeout = timeout;
  }

  public int getRangeOfAddress() {
    return rangeOfAddress;
  }

  public void setRangeOfAddress(int rangeOfAddress) throws AdminException {
    this.rangeOfAddress = rangeOfAddress;
  }

  public String getName() {
    return name;
  }

  public String getHost() {
    return host;
  }

  public void setHost(String host) throws AdminException {
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

  public String getSystemId() {
    return systemId;
  }

  public void setSystemId(String systemId) throws AdminException {
    vh.checkNotEmpty("systemId", systemId);
    this.systemId = systemId;
  }

  public String getPassword() {
    return password;
  }

  public void setPassword(String password) throws AdminException {
    vh.checkNotNull("password", password);
    this.password = password;
  }

  public String getSystemType() {
    return systemType;
  }

  public void setSystemType(String systemType) throws AdminException {
    vh.checkNotEmpty("systemType", systemType);
    this.systemType = systemType;
  }

  public int getInterfaceVersion() {
    return interfaceVersion;
  }

  public void setInterfaceVersion(int interfaceVersion) throws AdminException {
    vh.checkGreaterOrEqualsTo("interfaceVersion", interfaceVersion, 0);
    this.interfaceVersion = interfaceVersion;
  }

  public int getUssdServiceOp() {
    return ussdServiceOp;
  }

  public void setUssdServiceOp(int ussdServiceOp) throws AdminException {
    vh.checkGreaterOrEqualsTo("ussdServiceOp", ussdServiceOp, 0);
    this.ussdServiceOp = ussdServiceOp;
  }

  public int getVlrUssdServiceOp() {
    return vlrUssdServiceOp;
  }

  public void setVlrUssdServiceOp(int vlrUssdServiceOp) throws AdminException {
    vh.checkGreaterOrEqualsTo("vlrUssdServiceOp", vlrUssdServiceOp, 0);
    this.vlrUssdServiceOp = vlrUssdServiceOp;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Smsc smsc = (Smsc) o;

    if (defaultValidityPeriod != smsc.defaultValidityPeriod) return false;
    if (interfaceVersion != smsc.interfaceVersion) return false;
    if (maxValidityPeriod != smsc.maxValidityPeriod) return false;
    if (minValidityPeriod != smsc.minValidityPeriod) return false;
    if (port != smsc.port) return false;
    if (rangeOfAddress != smsc.rangeOfAddress) return false;
    if (timeout != smsc.timeout) return false;
    if (ussdServiceOp != smsc.ussdServiceOp) return false;
    if (vlrUssdServiceOp != smsc.vlrUssdServiceOp) return false;
    if (host != null ? !host.equals(smsc.host) : smsc.host != null) return false;
    if (name != null ? !name.equals(smsc.name) : smsc.name != null) return false;
    if (password != null ? !password.equals(smsc.password) : smsc.password != null) return false;
    if (systemId != null ? !systemId.equals(smsc.systemId) : smsc.systemId != null) return false;
    if (systemType != null ? !systemType.equals(smsc.systemType) : smsc.systemType != null) return false;

    if (permanentErrors.size() != smsc.permanentErrors.size()) {
      return false;
    } else {
      for (Integer e1 : permanentErrors) {
        boolean exist = false;
        for (Integer e2 : smsc.permanentErrors) {
          if (e1.equals(e2)) {
            exist = true;
            break;
          }
        }
        if (!exist) {
          return false;
        }
      }
    }
    if (immediateErrors.size() != smsc.immediateErrors.size()) {
      return false;
    } else {
      for (Integer e1 : immediateErrors) {
        boolean exist = false;
        for (Integer e2 : smsc.immediateErrors) {
          if (e1.equals(e2)) {
            exist = true;
            break;
          }
        }
        if (!exist) {
          return false;
        }
      }
    }
    if (temporaryErrors.size() != smsc.temporaryErrors.size()) {
      return false;
    } else {
      for (Map.Entry<String, Collection<Integer>> e1 : temporaryErrors.entrySet()) {
        if (!smsc.temporaryErrors.containsKey(e1.getKey())) {
          return false;
        }
        for (Integer ee1 : e1.getValue()) {
          boolean exist = false;
          for (Integer ee2 : smsc.temporaryErrors.get(e1.getKey())) {
            if (ee1.equals(ee2)) {
              exist = true;
              break;
            }
          }
          if (!exist) {
            return false;
          }
        }
      }
    }
    return true;
  }

  @Override
  public int hashCode() {
    return 0;
  }

  public Smsc cloneSmsc() {
    Smsc s = new Smsc();
    s.name = name;
    s.host = host;
    s.port = port;
    s.systemId = systemId;
    s.password = password;
    s.systemType = systemType;
    s.interfaceVersion = interfaceVersion;
    s.ussdServiceOp = ussdServiceOp;
    s.vlrUssdServiceOp = vlrUssdServiceOp;
    s.timeout = timeout;
    s.rangeOfAddress = rangeOfAddress;
    s.defaultValidityPeriod = defaultValidityPeriod;
    s.maxValidityPeriod = maxValidityPeriod;
    s.minValidityPeriod = minValidityPeriod;
    s.immediateErrors.addAll(immediateErrors);
    s.permanentErrors.addAll(permanentErrors);
    for (Map.Entry<String, Collection<Integer>> e : temporaryErrors.entrySet()) {
      s.temporaryErrors.put(e.getKey(), new HashSet<Integer>(e.getValue()));
    }
    return s;
  }

}
