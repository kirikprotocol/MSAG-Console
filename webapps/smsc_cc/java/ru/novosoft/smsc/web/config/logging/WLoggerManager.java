package ru.novosoft.smsc.web.config.logging;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.logging.Logger;
import ru.novosoft.smsc.admin.logging.LoggerManager;
import ru.novosoft.smsc.admin.logging.LoggerSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.config.DiffHelper;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.lang.reflect.Method;
import java.util.Collection;
import java.util.List;

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
        j.addRecord(JournalRecord.Type.REMOVE, JournalRecord.Subject.LOGGING, user).setDescription("logger_removed", oldName);
      } else {
        List<Method> getters = DiffHelper.getGetters(Logger.class);
        List<Object> oldValues = DiffHelper.callGetters(getters, oldLogger);
        List<Object> newValues = DiffHelper.callGetters(getters, newLogger);
        DiffHelper.logChanges(j, JournalRecord.Subject.LOGGING, oldValues, newValues, getters, user, "logger_property_changed", oldName);
      }
    }

    for (String newName : newSettings.getNames()) {
      if (!oldNames.contains(newName))
        j.addRecord(JournalRecord.Type.ADD, JournalRecord.Subject.LOGGING, user).setDescription("logger_added", newName);
    }
  }

  public LoggerSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public LoggerSettings cloneSettings(LoggerSettings settings) {
    return settings.cloneSettings();
  }
}
