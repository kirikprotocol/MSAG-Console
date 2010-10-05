package ru.novosoft.smsc.web.config.logging;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.logging.Logger;
import ru.novosoft.smsc.admin.logging.LoggerManager;
import ru.novosoft.smsc.admin.logging.LoggerSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;

import static ru.novosoft.smsc.web.config.DiffHelper.*;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public class WLoggerManager extends BaseSettingsManager<LoggerSettings> implements LoggerManager {
  
  private final LoggerManager wrapped;
  private final Journal j;

  public WLoggerManager(LoggerManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }

  @Override
  protected void _updateSettings(LoggerSettings newSettings) throws AdminException {
    LoggerSettings oldSettings = getSettings();
    wrapped.updateSettings(newSettings);
    
    Collection<String> oldNames = oldSettings.getNames();
    for (String oldName : oldNames) {
      Logger oldLogger = oldSettings.getLogger(oldName);
      Logger newLogger = newSettings.getLogger(oldName);
      if (newLogger == null) {
        j.user(user).remove().logger(oldName);
      } else {
        final String loggerName = oldName;
        findChanges(oldLogger, newLogger, Logger.class, new ChangeListener() {
          public void foundChange(String propertyName, Object oldValue, Object newValue) {
            j.user(user).change("property_changed", valueToString(oldValue), valueToString(newValue)).logger(loggerName);
          }
        });
      }
    }

    for (String newName : newSettings.getNames()) {
      if (!oldNames.contains(newName))
        j.user(user).add().logger(newName);        
    }
  }

  public LoggerSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public LoggerSettings cloneSettings(LoggerSettings settings) {
    return settings.cloneSettings();
  }
}
