package ru.novosoft.smsc.web.config.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.reschedule.Reschedule;
import ru.novosoft.smsc.admin.reschedule.RescheduleManager;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class WRescheduleManager extends BaseSettingsManager<RescheduleSettings> implements RescheduleManager {

  private final RescheduleManager wrapped;
  private final Journal j;

  public WRescheduleManager(RescheduleManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }

  public RescheduleSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  @Override
  protected void _updateSettings(RescheduleSettings settings) throws AdminException {
    RescheduleSettings oldSettings = getSettings();
    wrapped.updateSettings(settings);
    logChanges(oldSettings, settings);
  }

  public RescheduleSettings cloneSettings(RescheduleSettings settings) {
    return settings.cloneSettings();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }

  /**
   * По коллекции политик расписаний строит Map, ключем в котором является SMPP status, значением - политика
   * передоставки для этого статуса.
   *
   * @param reschedules список политик передоставки
   * @return Map, ключем в котором является SMPP status, значением - политика передоставки для этого статуса.
   */
  private static Map<Integer, String> expandReschedules(Collection<Reschedule> reschedules) {
    Map<Integer, String> result = new HashMap<Integer, String>();
    for (Reschedule r : reschedules) {
      for (int status : r.getStatuses())
        result.put(status, r.getIntervals());
    }
    return result;
  }

  public void logChanges(RescheduleSettings oldSettings, RescheduleSettings newSettings) {
    Map<Integer, String> oldReschedules = expandReschedules(oldSettings.getReschedules());
    Map<Integer, String> newReschedules = expandReschedules(newSettings.getReschedules());

    for (Map.Entry<Integer, String> e : oldReschedules.entrySet()) {
      int smppStatus = e.getKey();
      String oldReschedule = e.getValue();
      String newReschedule = newReschedules.get(smppStatus);
      if (newReschedule == null)
        j.user(user).remove().reschedule(String.valueOf(smppStatus));
      else if (!newReschedule.equals(oldReschedule))
        j.user(user).change("changed", oldReschedule, newReschedule).reschedule(String.valueOf(smppStatus));
    }

    for (Map.Entry<Integer, String> e : newReschedules.entrySet()) {
      int smppStatus = e.getKey();
      if (!oldReschedules.containsKey(smppStatus))
        j.user(user).add().reschedule(String.valueOf(smppStatus), e.getValue());
    }

    if (!oldSettings.getDefaultReschedule().equals(newSettings.getDefaultReschedule()))
      j.user(user).change("change_default", oldSettings.getDefaultReschedule(), newSettings.getDefaultReschedule()).reschedule();
  }
}
