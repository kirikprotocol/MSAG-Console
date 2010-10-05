package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;

/**
 * @author Artem Snopkov
 */
public interface RescheduleManager extends SmscConfiguration {
  
  RescheduleSettings getSettings() throws AdminException;

  void updateSettings(RescheduleSettings settings) throws AdminException;
}
