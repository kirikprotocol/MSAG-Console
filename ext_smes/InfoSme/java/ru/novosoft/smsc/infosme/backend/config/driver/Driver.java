package ru.novosoft.smsc.infosme.backend.config.driver;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.AdminException;

/**
 * User: artem
 * Date: 26.01.2009
 */
public class Driver {

  private String type;
  private String loadup;

  private boolean modified;

  public Driver() {
    modified = true;
  }

  Driver(String name, Config config) throws AdminException {
    final String prefix = DriverManager.DRIVER_PREFIX + '.' + name;
    try {
      type = config.getString(prefix + ".type");
      loadup = config.getString(prefix + ".loadup");
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
    modified = false;
  }

  void storeToConfig(Config config) {
    final String prefix = DriverManager.DRIVER_PREFIX + '.' + type;
    config.setString(prefix + ".type", type);
    config.setString(prefix + ".loadup", loadup);
  }

  public String getLoadup() {
    return loadup;
  }

  public void setLoadup(String loadup) {
    this.loadup = loadup;
    setModified(true);
  }

  public boolean isModified() {
    return modified;
  }

  public void setModified(boolean modified) {
    this.modified = modified;
  }

  public String getType() {
    return type;
  }

  public void setType(String type) {
    this.type = type;
    setModified(true);
  }
}
