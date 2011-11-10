package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Time;

import java.util.Properties;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelSettings {

  private final ValidationHelper vh = new ValidationHelper(this.getClass());

  private String dbType;
  private String jdbcSource;
  private String jdbcLogin;
  private boolean removeOnStop;
  private String jdbcPassword;
  private int timeout;
  private int statsPeriod;
  private String user;
  private Time defValidityPeriod;
  private Time maxValidityPeriod;
  private Time minValidityPeriod;
  private int minPriority;
  private int maxPriority;

  private Properties properties = new Properties();

  public void validate() throws AdminException {
    vh.checkNotEmpty("jdbc.pool.type", dbType);
    vh.checkNotEmpty("jdbc.user", jdbcLogin);
    vh.checkNotEmpty("jdbc.source", jdbcSource);
    vh.checkNotNull("jdbc.password", jdbcPassword);
    vh.checkNotEmpty("user", user);
    vh.checkGreaterThan("timeout", timeout, 0);
    vh.checkGreaterThan("statsPeriod", statsPeriod, 0);
    vh.checkNotNull("validityPeriod.def", defValidityPeriod);
    vh.checkGreaterOrEqualsTo("validityPeriod.def", defValidityPeriod, new Time(0, 0, 20));
    vh.checkNotNull("validityPeriod.min", minValidityPeriod);
    vh.checkGreaterOrEqualsTo("validityPeriod.min", minValidityPeriod, new Time(0, 0, 20));
    vh.checkNotNull("validityPeriod.max", maxValidityPeriod);
    vh.checkGreaterOrEqualsTo("validityPeriod.max", maxValidityPeriod, new Time(0, 0, 20));
    vh.checkGreaterOrEqualsTo("validityPeriod.max", maxValidityPeriod, minValidityPeriod);
    vh.checkBetween("validityPeriod.def", defValidityPeriod, minValidityPeriod, maxValidityPeriod);

    vh.checkBetween("priority.min", minPriority, 1, 1000);
    vh.checkBetween("priority.max", maxPriority, 1, 1000);
    vh.checkGreaterOrEqualsTo("priority.max", maxPriority, minPriority);
  }

  public SiebelSettings(Properties properties) throws AdminException{
    this.properties.putAll(properties);
    load();
  }

  public SiebelSettings(SiebelSettings s) throws AdminException {
    this(s.getAllProperties());
  }

  private void load() throws AdminException{
    dbType = properties.getProperty("jdbc.pool.type");
    removeOnStop = Boolean.valueOf(properties.getProperty("removeOnStop"));
    jdbcLogin = properties.getProperty("jdbc.user");
    jdbcSource = properties.getProperty("jdbc.source");
    user = properties.getProperty("siebelUser");
    jdbcPassword = properties.getProperty("jdbc.password");
    try{
      statsPeriod = Integer.parseInt(properties.getProperty("statsPeriod"));
    }catch (NumberFormatException e) {
      throw new SiebelException("illegal_stats_period");
    }
    try{
      timeout = Integer.parseInt(properties.getProperty("timeout"));
    }catch (NumberFormatException e) {
      throw new SiebelException("illegal_timeout");
    }
    String s = properties.getProperty("validityPeriod.def");
    if(s != null && (s = s.trim()).length()>0) {
      try{
        defValidityPeriod = new Time(s);
      }catch (IllegalArgumentException e) {
        throw new SiebelException("illegal_validity_period");
      }
    }else {
      defValidityPeriod = new Time("01:00:00");
    }
    s = properties.getProperty("validityPeriod.max");
    if(s != null && (s = s.trim()).length()>0) {
      try{
        maxValidityPeriod = new Time(s);
      }catch (IllegalArgumentException e) {
        throw new SiebelException("illegal_validity_period");
      }
    }else {
      maxValidityPeriod = new Time("12:00:00");
    }

    s = properties.getProperty("validityPeriod.min");
    if(s != null && (s = s.trim()).length()>0) {
      try{
        minValidityPeriod = new Time(s);
      }catch (IllegalArgumentException e) {
        throw new SiebelException("illegal_validity_period");
      }
    }else {
      minValidityPeriod = new Time("00:00:20");
    }

    s = properties.getProperty("priority.max");
    if(s != null && (s = s.trim()).length() > 0) {
      try{
        maxPriority = Integer.parseInt(s);
      }catch (NumberFormatException e) {
        throw new SiebelException("illegal_priority");
      }
    }else {
      maxPriority = 1000;
    }

    s = properties.getProperty("priority.min");
    if(s != null && (s = s.trim()).length() > 0) {
      try{
        minPriority = Integer.parseInt(s);
      }catch (NumberFormatException e) {
        throw new SiebelException("illegal_priority");
      }
    }else {
      minPriority = 1;
    }
  }

  private void save(){
    properties.setProperty("jdbc.pool.type", dbType);
    properties.setProperty("removeOnStop", Boolean.toString(removeOnStop));
    properties.setProperty("jdbc.user", jdbcLogin);
    properties.getProperty("jdbc.source", jdbcSource);
    properties.setProperty("siebelUser", user);
    properties.setProperty("jdbc.password", jdbcPassword);
    properties.setProperty("statsPeriod", Integer.toString(statsPeriod));
    properties.setProperty("timeout", Integer.toString(timeout));
    properties.setProperty("validityPeriod.def", defValidityPeriod == null ? "01:00:00" : defValidityPeriod.toString());
    properties.setProperty("validityPeriod.max", maxValidityPeriod == null ? "12:00:00" : maxValidityPeriod.toString());
    properties.setProperty("validityPeriod.min", minValidityPeriod == null ? "00:00:20" : minValidityPeriod.toString());
    properties.setProperty("priority.min", Integer.toString(minPriority));
    properties.setProperty("priority.max", Integer.toString(maxPriority));
  }

  public Properties getAllProperties() {
    save();
    Properties p = new Properties();
    p.putAll(properties);
    return p;
  }


  public String getDbType() {
    return dbType;
  }

  public void setDbType(String dbType) {
    this.dbType = dbType;
  }

  public String getJdbcSource() {
    return jdbcSource;
  }

  public void setJdbcSource(String jdbcSource) {
    this.jdbcSource = jdbcSource;
  }

  public String getJdbcLogin() {
    return jdbcLogin;
  }

  public void setJdbcLogin(String jdbcLogin) {
    this.jdbcLogin = jdbcLogin;
  }

  public boolean isRemoveOnStop() {
    return removeOnStop;
  }

  public void setRemoveOnStop(boolean removeOnStop) {
    this.removeOnStop = removeOnStop;
  }

  public String getJdbcPassword() {
    return jdbcPassword;
  }

  public void setJdbcPassword(String jdbcPassword) {
    this.jdbcPassword = jdbcPassword;
  }

  public int getTimeout() {
    return timeout;
  }

  public void setTimeout(int timeout) {
    this.timeout = timeout;
  }

  public int getStatsPeriod() {
    return statsPeriod;
  }

  public void setStatsPeriod(int statsPeriod) {
    this.statsPeriod = statsPeriod;
  }

  public String getUser() {
    return user;
  }

  public void setUser(String user) {
    this.user = user;
  }

  public Time getDefValidityPeriod() {
    return defValidityPeriod;
  }

  public void setDefValidityPeriod(Time defValidityPeriod) {
    this.defValidityPeriod = defValidityPeriod;
  }

  public Time getMaxValidityPeriod() {
    return maxValidityPeriod;
  }

  public void setMaxValidityPeriod(Time maxValidityPeriod) {
    this.maxValidityPeriod = maxValidityPeriod;
  }

  public Time getMinValidityPeriod() {
    return minValidityPeriod;
  }

  public void setMinValidityPeriod(Time minValidityPeriod) {
    this.minValidityPeriod = minValidityPeriod;
  }

  public int getMinPriority() {
    return minPriority;
  }

  public void setMinPriority(int minPriority) {
    this.minPriority = minPriority;
  }

  public int getMaxPriority() {
    return maxPriority;
  }

  public void setMaxPriority(int maxPriority) {
    this.maxPriority = maxPriority;
  }
}
