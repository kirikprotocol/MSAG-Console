package ru.novosoft.smsc.web.controllers.logging;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.logging.Logger;
import ru.novosoft.smsc.admin.logging.LoggerSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import java.io.Serializable;
import java.util.*;

/**
 * author: alkhal
 */
public class LoggingController extends SettingsMController<LoggerSettings> {

  private static final org.apache.log4j.Logger logger = org.apache.log4j.Logger.getLogger(LoggingController.class);

  private HierarchicalLogger rLogger;
  private boolean initFailed;

  public LoggingController() {
    super(WebContext.getInstance().getLoggerManager());

    try {
      init();
    } catch (AdminException e) {
      addError(e);
      initFailed = true;
      return;
    }

    if (getRequestParameter("revision") == null) {
      _init();
    }
  }

  public boolean isInitFailed() {
    return initFailed;
  }

  private void _init() {
    LoggerSettings s = getSettings();
    Collection<String> logCategories = s.getNames();
    String rootPriority = logCategories.contains("") && s.getLogger("").getLevel() != null ? s.getLogger("").getLevel().toString() : null;
    if (rootPriority == null) rootPriority = "NOTSET";
    rLogger = new HierarchicalLogger("", "", rootPriority);
    Collection<String> keys = new TreeSet<String>(logCategories);
    for (String key : keys) {
      if (key.equals("")) {
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
      _init();
    } catch (AdminException e) {
      addError(e);
    }
    return "LOGGING";
  }

  private void convert(HierarchicalLogger logger, Map<String, Logger> loggers) {
    Logger l = new Logger();
    if (!logger.getLevel().equals("NOTSET")) {
      l.setLevel(Logger.Level.valueOf(logger.getLevel()));
    }
    if (LoggingController.logger.isDebugEnabled())
      LoggingController.logger.debug("CONVERT " + logger.getFullName() + " => " + logger.getLevel());
    loggers.put(logger.getFullName(), l);
    for (HierarchicalLogger c : logger.getChildList()) {
      convert(c, loggers);
    }

  }

  public String save() {
    try {
      Map<String, Logger> loggers = new HashMap<String, Logger>();
      convert(rLogger, loggers);
      if (logger.isDebugEnabled()) {
        for (Map.Entry<String, Logger> e : loggers.entrySet())
          logger.debug("Save: " + e.getKey() + " => " + e.getValue().getLevel());
      }
      setSettings(new LoggerSettings(loggers));
      submitSettings();

      return "INDEX";

    } catch (AdminException e) {
      logger.warn(e, e);
      addError(e);
      return null;
    }
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
      System.out.println("SET LEVEL : " + fullName + " => " + level);
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
      Collection<HierarchicalLogger> l = childs.values();
      if(l.isEmpty()) {
        return l;
      }
      List<HierarchicalLogger> list = new ArrayList<HierarchicalLogger>(childs.values());
      Collections.sort(list, new Comparator<HierarchicalLogger>() {
        public int compare(HierarchicalLogger o1, HierarchicalLogger o2) {
          boolean hasChild1 = !o1.childs.isEmpty();
          boolean hasChild2 = !o2.childs.isEmpty();
          if((hasChild1 && hasChild2) || !(hasChild1 || hasChild2)) {
            return o1.getName().compareTo(o2.getName());
          }else if (hasChild1) {
            return 1;
          }else {
            return -1;
          }
        }
      });
      return list;
    }
  }
}
