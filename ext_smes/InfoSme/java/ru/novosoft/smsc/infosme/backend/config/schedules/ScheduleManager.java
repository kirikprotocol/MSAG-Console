package ru.novosoft.smsc.infosme.backend.config.schedules;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.config.Changes;
import ru.novosoft.smsc.util.config.Config;

import java.util.*;

/**
 * User: artem
 * Date: 26.01.2009
 */
public class ScheduleManager {

  static final String SCHEDULES_PREFIX = "InfoSme.Schedules";

  private final Map schedules = new HashMap();
  private boolean modified = false;

  public ScheduleManager(Config cfg) throws AdminException {
    resetSchedules(cfg);
  }

  private List loadSchedules(Config config) throws AdminException {
    List result = new ArrayList(10);
    try {
      for (Iterator i = config.getSectionChildShortSectionNames(SCHEDULES_PREFIX).iterator(); i.hasNext();)
        result.add(Schedule.getInstance((String)i.next(), config));
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
    return result;
  }

  public synchronized void addSchedule(Schedule s) {
    schedules.put(s.getId(), s);
  }

  public synchronized void removeSchedule(String id) {
    schedules.remove(id);
    modified = true;
  }

  public synchronized Schedule getSchedule(String id) {
    return (Schedule)schedules.get(id);
  }

  public synchronized List getSchedules() {
    return new ArrayList(schedules.values());
  }

  public synchronized boolean containsSchedule(String id) {
    return schedules.containsKey(id);
  }

  public synchronized boolean isSchedulesChanged() {
    if (modified)
      return true;
    for (Iterator iter = schedules.values().iterator(); iter.hasNext();) {
      Schedule s = (Schedule)iter.next();
      if (s.isModified())
        return true;
    }
    return false;
  }

  private Changes analyzeChanges(List oldSchedules) {
    Changes changes = new Changes();

    // Lookup new schedules
    for (Iterator iter = schedules.keySet().iterator(); iter.hasNext();) {
      String id = (String)iter.next();

      boolean contains = false;
      for (Iterator iter1 = oldSchedules.iterator(); iter1.hasNext();) {
        Schedule t = (Schedule)iter1.next();
        if (t.getId().equals(id)) {
          contains = true;
          break;
        }
      }

      if (!contains)
        changes.added(id);
    }

    // Lookup deleted schedules
    for (Iterator iter = oldSchedules.iterator(); iter.hasNext();) {
      Schedule t = (Schedule)iter.next();
      if (!containsSchedule(t.getId()))
        changes.deleted(t.getId());
    }

    // Lookup modified schedules
    for (Iterator iter = schedules.values().iterator(); iter.hasNext();) {
      Schedule t = (Schedule)iter.next();
      if (t.isModified() && !changes.isAdded(t.getId()) && !changes.isDeleted(t.getId()))
        changes.modified(t.getId());
    }

    return changes;
  }

  public synchronized Changes applySchedules(Config cfg) throws AdminException {
    try {
      List oldSchedules = loadSchedules(cfg);

      Changes changes = analyzeChanges(oldSchedules);

      // Remove old schedules from config
      cfg.removeSection(SCHEDULES_PREFIX);

      // Add new schedules to config
      for (Iterator iter = schedules.values().iterator(); iter.hasNext();) {
        Schedule s = (Schedule)iter.next();
        s.storeToConfig(cfg);
      }

      return changes;
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public void setModified(boolean modified) {
    this.modified = modified;
    for (Iterator iter = schedules.values().iterator(); iter.hasNext();) {
        Schedule s = (Schedule)iter.next();
        s.setModified(false);
      }
  }

  public synchronized void resetSchedules(Config cfg) throws AdminException {
    try {
      List oldSchedules = loadSchedules(cfg);
      schedules.clear();
      for (Iterator iter = oldSchedules.iterator(); iter.hasNext();) {
        Schedule s = (Schedule)iter.next();
        schedules.put(s.getId(), s);
      }
      modified = false;
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

}
