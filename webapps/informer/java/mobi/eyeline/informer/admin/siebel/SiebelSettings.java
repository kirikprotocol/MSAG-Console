package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

import java.util.Properties;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelSettings {

  private final ValidationHelper vh =
      new ValidationHelper(this.getClass());

  private final Properties properties;

  public SiebelSettings(Properties properties) {
    this.properties = properties;
  }

  public SiebelSettings(SiebelSettings s) {
    this(s.getAllProperties());
  }

  public void validate() throws AdminException{
    vh.checkNotEmpty("dbType", getDbType());
    vh.checkNotEmpty("jdbcLogin", getJdbcLogin());
    vh.checkNotEmpty("jdbcSource", getJdbcSource());
    vh.checkNotEmpty("jdbcPassword", getJdbcPassword());
    vh.checkNotEmpty("user", getUser());
    vh.checkGreaterThan("timeout", getTimeout(), 0);
    vh.checkGreaterThan("statsPeriod", getStatsPeriod(), 0);
  }

  public Properties getAllProperties() {
    Properties p = new Properties();
    p.putAll(properties);
    return p;
  }

  public int getTimeout() {
    String p = properties.getProperty("timeout");
    return p == null || p.length() == 0 ? 0 : Integer.parseInt(p);
  }

  public void setTimeout(int timeout) {
    properties.setProperty("timeout", Integer.toString(timeout));
  }

  public int getStatsPeriod() {
    String p = properties.getProperty("statsPeriod");
    return p == null || p.length() == 0 ? 0 : Integer.parseInt(p);
  }

  public void setStatsPeriod(int statsPeriod) {
    properties.setProperty("statsPeriod", Integer.toString(statsPeriod));
  }

  public String getDbType() {
    return properties.getProperty("jdbc.pool.type");
  }

  public void setDbType(String dbType) {
    if(dbType != null) {
      properties.setProperty("jdbc.pool.type", dbType);
    }else {
      properties.remove("jdbc.pool.type");
    }
  }

  public String getJdbcSource() {
    return properties.getProperty("jdbc.source");
  }

  public void setJdbcSource(String jdbcSource) {
    if(jdbcSource != null) {
      properties.setProperty("jdbc.source", jdbcSource);
    }else {
      properties.remove("jdbc.source");
    }
  }

  public String getUser() {
    return properties.getProperty("siebelUser");
  }

  public void setUser(String user) {
    if(user != null) {
      properties.setProperty("siebelUser", user);
    }else {
      properties.remove("siebelUser");
    }
  }

  public String getJdbcLogin() {
    return properties.getProperty("jdbc.user");
  }

  public void setJdbcLogin(String jdbcLogin) {
    if(jdbcLogin != null) {
      properties.setProperty("jdbc.user", jdbcLogin);
    }else {
      properties.remove("jdbc.user");
    }
  }

  public String getJdbcPassword() {
    return properties.getProperty("jdbc.password");
  }

  public void setJdbcPassword(String jdbcPassword) {
    if(jdbcPassword != null) {
      properties.setProperty("jdbc.password", jdbcPassword);
    }else {
      properties.remove("jdbc.password");
    }
  }

  public boolean isRemoveOnStop() {
    return Boolean.valueOf(properties.getProperty("removeOnStop"));
  }

  public void setRemoveOnStop(boolean removeOnStop) {
    properties.setProperty("removeOnStop", Boolean.toString(removeOnStop));
  }
}
