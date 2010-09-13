package ru.novosoft.smsc.web.controllers.logging;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.logging.Logger;
import ru.novosoft.smsc.admin.logging.LoggerSettings;
import ru.novosoft.smsc.web.controllers.SettingsController;

import javax.faces.application.FacesMessage;
import java.io.Serializable;
import java.util.*;

/**
 * author: alkhal
 */
public class LoggingController extends SettingsController<LoggerSettings> {

  private static final org.apache.log4j.Logger logger = org.apache.log4j.Logger.getLogger(LoggingController.class);

  private HierarchicalLogger rLogger;

  public LoggingController() {
    super(ConfigType.Logger);

    if (isSettingsChanged())
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.configuration.locally.changed");

    if(getRequestParameter("revision") == null) {
      System.out.println("INIT LOGGING");
      init();
    }
  }

  private void init() {
    LoggerSettings s = getSettings();
    Collection<String> logCategories = s.getNames();
    String rootPriority = logCategories.contains("") && s.getLogger("").getLevel() != null ? s.getLogger("").getLevel().toString() : null;
    if (rootPriority == null) rootPriority = "NOTSET";
    rLogger = new HierarchicalLogger("", "", rootPriority);
    Collection<String> keys = new TreeSet<String>(logCategories);
    for (String key : keys) {
      if(key.equals("")) {
        continue;
      }
      Logger.Level l = s.getLogger(key).getLevel();
      String value = l == null ? "NOTSET" : l.toString();
      rLogger.addChild(key, value);
    }
  }

  public HierarchicalLogger getrLogger() {
    return rLogger;
  }

  public void setrLogger(HierarchicalLogger rLogger) {
    this.rLogger = rLogger;
  }

  public String reset() {
    try {
      resetSettings();
      init();
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  private void convert(HierarchicalLogger logger, Map<String, Logger> loggers) {
    Logger l = new Logger();
    if (!logger.getLevel().equals("NOTSET")) {
      l.setLevel(Logger.Level.valueOf(logger.getLevel()));
    }
    System.out.println("CONVERT "+logger.getFullName()+" => "+logger.getLevel());
    loggers.put(logger.getFullName(), l);
    for (HierarchicalLogger c : logger.getChildList()) {
      convert(c, loggers);
    }

  }

  public String save() {
    try {
      Map<String, Logger> loggers = new HashMap<String, Logger>();
      convert(rLogger, loggers);
      for(Map.Entry<String, Logger> e : loggers.entrySet()) {
        System.out.println("Save: "+e.getKey()+" => "+e.getValue().getLevel());
      }
      setSettings(new LoggerSettings(loggers));
      Revision rev = submitSettings();
      if (rev != null) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.not.actual", rev.getUser());
        return null;
      }

      return "INDEX";

    } catch (AdminException e) {
      logger.warn(e, e);
      addError(e);
      return null;
    }
  }

  @Override
  protected LoggerSettings loadSettings() throws AdminException {
    return getConfiguration().getLoggerSettings();
  }

  @Override
  protected void saveSettings(LoggerSettings settings) throws AdminException {
    getConfiguration().updateLoggerSettings(settings, getUserPrincipal().getName());
  }

  @Override
  protected LoggerSettings cloneSettings(LoggerSettings settings) {
    return settings.cloneSettings();
  }


  public static class HierarchicalLogger implements Serializable, Comparable {

    private String name;
    private String fullName;
    private String level;
    private Map<String, HierarchicalLogger> childs;

    public HierarchicalLogger() {
    }

    public HierarchicalLogger(String name, String fullName, String level) {
      this.name = name;
      this.fullName = fullName;
      this.level = level;
      this.childs = new TreeMap<String, HierarchicalLogger>();
    }

    public void addChild(String childFullName, String childPriority) {
      logger.debug("Adding child: name=\"" + fullName + "\" priority=" + level + ",  childName=\"" + childFullName + "\" childPriority=" + childPriority);
      if (isRoot()) {
        final int dotPos = childFullName.indexOf('.');
        if (dotPos > 0) {
          final String childName = childFullName.substring(0, dotPos);
          HierarchicalLogger child = getOrCreateChild(childName, childName, "NOTSET");
          child.addChild(childFullName, childPriority);
        } else {
          childs.put(childFullName, new HierarchicalLogger(childFullName, childFullName, childPriority));
        }
      } else {
        if (childFullName.length() > fullName.length() && childFullName.startsWith(fullName) && childFullName.charAt(fullName.length()) == '.') {
          final int beginIndex = fullName.length() + 1;
          final int endIndex = childFullName.indexOf('.', beginIndex);
          final String childName = endIndex > 0 ? childFullName.substring(beginIndex, endIndex) : childFullName.substring(beginIndex);
          if (endIndex > 0) {
            HierarchicalLogger child = getOrCreateChild(childName, childFullName.substring(0, endIndex), "NOTSET");
            child.addChild(childFullName, childPriority);
          } else {
            HierarchicalLogger child = getOrCreateChild(childName, childFullName, childPriority);
            child.level = childPriority;
          }
        } else {
          logger.debug("Incorrect LoggerCategoryInfo.addChild algorithm");
        }
      }
    }

    public boolean isRoot() {
      return fullName == null || fullName.length() == 0;
    }

    public boolean isHasChilds() {
      return !childs.isEmpty();
    }

    private HierarchicalLogger getOrCreateChild(String childName, String childFullName, String childPriority) {
      HierarchicalLogger child = childs.get(childName);
      if (child == null) {
        child = new HierarchicalLogger(childName, childFullName, childPriority);
        childs.put(childName, child);
      }
      return child;
    }

    public int compareTo(Object o) {
      if (o instanceof HierarchicalLogger) {
        HierarchicalLogger info = (HierarchicalLogger) o;
        return name.compareTo(info.name);
      } else
        return 0;
    }

    public String getName() {
      return name;
    }

    public String getFullName() {
      return fullName;
    }

    public String getLevel() {
      return level;
    }

    public void setLevel(String level) {
      System.out.println("SET LEVEL : "+fullName+" => "+level);
      this.level = level;
    }

    public Map<String, HierarchicalLogger> getChilds() {
      return childs;
    }

    public void setName(String name) {
      this.name = name;
    }

    public void setFullName(String fullName) {
      this.fullName = fullName;
    }

    public void setChilds(Map<String, HierarchicalLogger> childs) {
      this.childs = childs;
    }

    public Collection<HierarchicalLogger> getChildList() {
      return childs.values();
    }
  }
}
