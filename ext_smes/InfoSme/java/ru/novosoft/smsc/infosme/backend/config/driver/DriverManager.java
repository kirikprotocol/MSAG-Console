package ru.novosoft.smsc.infosme.backend.config.driver;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.config.Config;

import java.util.*;

/**
 * User: artem
 * Date: 26.01.2009
 */
public class DriverManager {

  public static final String DRIVER_PREFIX = "StartupLoader.DataSourceDrivers";

  private final Map drivers = new HashMap();
  private boolean modified;

  public DriverManager(Config cfg) throws AdminException {
    resetDrivers(cfg);
  }

  public synchronized void addDriver(Driver d) {
    drivers.put(d.getType(), d);
  }

  public synchronized void removeDriver(String type) {
    drivers.remove(type);
    modified = true;
  }

  public synchronized void clearDrivers() {
    drivers.clear();
    modified = true;
  }

  public synchronized Driver getDriver(String type) {
    return (Driver)drivers.get(type);
  }

  public synchronized List getDrivers() {
    return new ArrayList(drivers.values());
  }

  public synchronized boolean isDriversChanged() {
    if (modified)
      return true;
    for (Iterator iter = drivers.values().iterator(); iter.hasNext();) {
      Driver d = (Driver)iter.next();
      if (d.isModified())
        return true;
    }
    return false;
  }

  public synchronized void applyDrivers(Config cfg) throws AdminException {
    try {
      // Remove old drivers
      cfg.removeSection(DRIVER_PREFIX);

      // Add new drivers
      for (Iterator iter = drivers.values().iterator(); iter.hasNext();) {
        Driver d = (Driver)iter.next();
        d.storeToConfig(cfg);
      }

    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public void setModified(boolean modified) {
    this.modified = modified;
    for (Iterator iter = drivers.values().iterator(); iter.hasNext();) {
        Driver d = (Driver)iter.next();
        d.setModified(false);
      }
  }

  public synchronized void resetDrivers(Config cfg) throws AdminException {
    drivers.clear();
    try {
      for (Iterator iter = loadDrivers(cfg).iterator(); iter.hasNext();)
        addDriver((Driver)iter.next());
      modified = false;
    } catch (Exception e) {
      throw new AdminException(e.getMessage());
    }
  }

  private static List loadDrivers(Config config) throws AdminException {
    List result = new ArrayList(10);
    try {
      for (Iterator i = config.getSectionChildShortSectionNames(DRIVER_PREFIX).iterator(); i.hasNext();)
        result.add(new Driver((String)i.next(), config));
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
    return result;
  }
}
