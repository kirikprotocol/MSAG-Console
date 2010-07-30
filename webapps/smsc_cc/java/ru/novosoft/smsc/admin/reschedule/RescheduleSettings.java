package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class RescheduleSettings {

  private static final ValidationHelper vh = new ValidationHelper(RescheduleSettings.class);

  private String defaultReschedule;
  private int rescheduleLimit;
  private Collection<Reschedule> reschedules = new ArrayList<Reschedule>();

  RescheduleSettings() {
  }

  RescheduleSettings(RescheduleSettings c) {
    this.rescheduleLimit = c.rescheduleLimit;
    this.defaultReschedule = c.defaultReschedule;
    List<Reschedule> r = new ArrayList<Reschedule>(c.reschedules.size());
    for (Reschedule rr : c.reschedules)
      r.add(new Reschedule(rr));
    this.reschedules = r;
  }

  public void setRescheduleLimit(int rescheduleLimit) throws AdminException {
    vh.checkPositive("scheduleLimit", rescheduleLimit);
    this.rescheduleLimit = rescheduleLimit;
  }

  public int getRescheduleLimit() {
    return rescheduleLimit;
  }

  public String getDefaultReschedule() {
    return defaultReschedule;
  }

  public void setDefaultReschedule(String intervals) throws AdminException {
    vh.checkMaches("defaultReschedule", intervals, Reschedule.intervalsPattern);
    this.defaultReschedule = intervals;
  }

  public Collection<Reschedule> getReschedules() {
    return reschedules;
  }

  public void setReschedules(Collection<Reschedule> reschedules) throws AdminException {
    vh.checkNoNulls("reschedules", reschedules);
    for (Reschedule r1 : reschedules)
      for (Reschedule r2 : reschedules) {
        if (r1 != r2)
          vh.checkNotIntersect("reschedule_statuses", r1.getStatuses(), r2.getStatuses());
      }
    this.reschedules = reschedules;
  }

  public RescheduleSettings cloneSettings() {
    return new RescheduleSettings(this);
  }
}
