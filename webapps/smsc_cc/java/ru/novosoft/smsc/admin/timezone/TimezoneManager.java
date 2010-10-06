package ru.novosoft.smsc.admin.timezone;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;

/**
 * @author Artem Snopkov
 */
public interface TimezoneManager extends SmscConfiguration {
  TimezoneSettings getSettings() throws AdminException;

  void updateSettings(TimezoneSettings settings) throws AdminException;
}
