package ru.novosoft.smsc.admin.logging;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.CCLoggingInfo;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class LoggerManagerImpl implements LoggerManager {

  private final ClusterController cc;

  public LoggerManagerImpl(ClusterController cc) {
    this.cc = cc;
  }

  public LoggerSettings getSettings() throws AdminException {
    if (!cc.isOnline())
      throw new LoggerException("logger.info.unavailable");

    Collection<CCLoggingInfo> infos = cc.getLoggers();
    Map<String, Logger> loggers = new HashMap<String, Logger>();
    for (CCLoggingInfo info : infos) {
      Logger.Level l = null;
      if (info.getLevel() != null) {
        switch (info.getLevel()) {
          case DEBUG: l = Logger.Level.DEBUG; break;
          case INFO: l = Logger.Level.INFO; break;
          case WARN: l = Logger.Level.WARN; break;
          case ERROR: l = Logger.Level.ERROR; break;
          default : l = Logger.Level.FATAL;
        }
      }
      Logger logger = new Logger();
      logger.setLevel(l);
      loggers.put(info.getName(), logger);
    }
    return new LoggerSettings(loggers);
  }

  public void updateSettings(LoggerSettings settings) throws AdminException {

    if (!cc.isOnline())
      throw new LoggerException("logger.info.unavailable");

    Map<String, Logger> loggers = settings.getLoggers();
    Collection<CCLoggingInfo> infos = new ArrayList<CCLoggingInfo>();

    for (Map.Entry<String, Logger> e : loggers.entrySet()) {
        CCLoggingInfo info = new CCLoggingInfo();
        info.setName(e.getKey());
        if(e.getValue().getLevel() != null) {
          CCLoggingInfo.Level l;
          switch (e.getValue().getLevel()) {
            case DEBUG: l = CCLoggingInfo.Level.DEBUG; break;
            case INFO: l = CCLoggingInfo.Level.INFO; break;
            case WARN: l = CCLoggingInfo.Level.WARN; break;
            case ERROR: l = CCLoggingInfo.Level.ERROR; break;
            default: l = CCLoggingInfo.Level.FATAL;
          }
          info.setLevel(l);
        }
        infos.add(info);
    }
    cc.setLoggers(infos);
  }

}
