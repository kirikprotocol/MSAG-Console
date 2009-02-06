package ru.novosoft.smsc.infosme.backend.config.provider;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.admin.AdminException;

/**
 * User: artem
 * Date: 26.01.2009
 */
public class Provider {

  private String name;
  private String type;
  private int connections;
  private String dbInstance;
  private String dbUserName;
  private String dbUserPassword;
  private boolean watchdog;

  private boolean modified;

  public Provider() {
    modified = true;
  }

  Provider(String name, Config config) throws AdminException {
    final String prefix = ProviderManager.PROVIDER_PREFIX + '.' + StringEncoderDecoder.encodeDot(name);
    try {
      this.name = name;
      type = config.getString(prefix + ".type");
      connections = config.getInt(prefix + ".connections");
      dbInstance = config.getString(prefix + ".dbInstance");
      dbUserName = config.getString(prefix + ".dbUserName");
      dbUserPassword = config.getString(prefix + ".dbUserPassword");
      watchdog = config.getBool(prefix + ".watchdog");
      modified = false;
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  void storeToConfig(Config config) {
    final String prefix = ProviderManager.PROVIDER_PREFIX + '.' + StringEncoderDecoder.encodeDot(name);
    config.setString(prefix + ".type", type);
    config.setInt(prefix + ".connections", connections);
    config.setString(prefix + ".dbInstance", dbInstance);
    config.setString(prefix + ".dbUserName", dbUserName);
    config.setString(prefix + ".dbUserPassword", dbUserPassword);
    config.setBool(prefix + ".watchdog", watchdog);
  }

  public int getConnections() {
    return connections;
  }

  public void setConnections(int connections) {
    this.connections = connections;
    setModified(true);
  }

  public String getDbInstance() {
    return dbInstance;
  }

  public void setDbInstance(String dbInstance) {
    this.dbInstance = dbInstance;
    setModified(true);
  }

  public String getDbUserName() {
    return dbUserName;
  }

  public void setDbUserName(String dbUserName) {
    this.dbUserName = dbUserName;
    setModified(true);
  }

  public String getDbUserPassword() {
    return dbUserPassword;
  }

  public void setDbUserPassword(String dbUserPassword) {
    this.dbUserPassword = dbUserPassword;
    setModified(true);
  }

  public boolean isModified() {
    return modified;
  }

  public void setModified(boolean modified) {
    this.modified = modified;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
    setModified(true);
  }

  public String getType() {
    return type;
  }

  public void setType(String type) {
    this.type = type;
    setModified(true);
  }

  public boolean isWatchdog() {
    return watchdog;
  }

  public void setWatchdog(boolean watchdog) {
    this.watchdog = watchdog;
    setModified(true);
  }
}
