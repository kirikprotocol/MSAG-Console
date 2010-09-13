package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.logging.Logger;
import ru.novosoft.smsc.admin.logging.LoggerSettings;

import java.lang.reflect.Method;
import java.util.Collection;
import java.util.List;

/**
 * author: alkhal
 */
public class LoggerSettingsDiffHelper extends DiffHelper {

  public LoggerSettingsDiffHelper(String subject) {
    super(subject);
  }

  public void logChanges(Journal j, LoggerSettings oldSettings, LoggerSettings newSettings, String user) {
    Collection<String> oldNames = oldSettings.getNames();
    for (String oldName : oldNames) {
      Logger oldLogger = oldSettings.getLogger(oldName);
      Logger newLogger = newSettings.getLogger(oldName);
      if (newLogger == null) {
        j.addRecord(JournalRecord.Type.REMOVE, subject, user).setDescription("logger_removed", oldName);
      } else {
        List<Method> getters = getGetters(Logger.class);
        List<Object> oldValues = callGetters(getters, oldLogger);
        List<Object> newValues = callGetters(getters, newLogger);
        logChanges(j, oldValues, newValues, getters, user, "logger_property_changed", oldName);
      }
    }
    for (String newName : newSettings.getNames()) {
      if (!oldNames.contains(newName)) {
        j.addRecord(JournalRecord.Type.ADD, subject, user).setDescription("logger_added", newName);
      }
    }
  }
}
