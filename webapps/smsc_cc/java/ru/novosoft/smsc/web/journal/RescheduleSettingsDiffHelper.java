package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.reschedule.Reschedule;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
class RescheduleSettingsDiffHelper extends DiffHelper {

  public RescheduleSettingsDiffHelper(String subject) {
    super(subject);
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

  public void logChanges(Journal j, RescheduleSettings oldSettings, RescheduleSettings newSettings, String user) {
    Map<Integer, String> oldReschedules = expandReschedules(oldSettings.getReschedules());
    Map<Integer, String> newReschedules = expandReschedules(newSettings.getReschedules());

    for (Map.Entry<Integer, String> e : oldReschedules.entrySet()) {
      int smppStatus = e.getKey();
      String oldReschedule = e.getValue();
      String newReschedule = newReschedules.get(smppStatus);
      if (newReschedule == null)
        j.addRecord(JournalRecord.Type.REMOVE, subject, user).setDescription("reschedule_removed", String.valueOf(smppStatus));
      else if (!newReschedule.equals(oldReschedule))
        j.addRecord(JournalRecord.Type.CHANGE, subject, user).setDescription("reschedule_changed", String.valueOf(smppStatus), oldReschedule, newReschedule);
    }

    for (Map.Entry<Integer, String> e : newReschedules.entrySet()) {
      int smppStatus = e.getKey();
      if (!oldReschedules.containsKey(smppStatus))
        j.addRecord(JournalRecord.Type.ADD, subject, user).setDescription("reschedule_added", String.valueOf(smppStatus), e.getValue());
    }

    if (!oldSettings.getDefaultReschedule().equals(newSettings.getDefaultReschedule()))
      j.addRecord(JournalRecord.Type.CHANGE, subject, user).setDescription("property_changed", "defaultReschedule", oldSettings.getDefaultReschedule(), newSettings.getDefaultReschedule());
  }
}
