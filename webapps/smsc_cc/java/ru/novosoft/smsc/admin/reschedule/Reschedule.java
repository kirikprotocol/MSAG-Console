package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.util.ArrayList;
import java.util.Collection;
import java.util.regex.Pattern;

/**
 * @author Artem Snopkov
 */
public class Reschedule {

  private static final ValidationHelper vh = new ValidationHelper(Reschedule.class.getCanonicalName());

  private static final String LAST = "(\\d\\d{0,2}(m|h|s|d)(|:\\d{1,4}|:\\*))";
  private static final String MEDIUM = "(\\d\\d{0,2}(m|h|s|d)(|:\\d{1,4}))";
  public static final Pattern intervalsPattern=Pattern.compile("(" + LAST + "|" +  MEDIUM + "(," + MEDIUM + ")*" + "(," + LAST + ")?" + ")");

  private final Collection<Integer> statuses;
  private final String intervals;

  public Reschedule(String intervals, int... statuses)  {
    if (intervals == null)
      throw new IllegalArgumentException("Intervals are null");

    if (!intervalsPattern.matcher(intervals).matches())
      throw new IllegalArgumentException("intervals");

    this.intervals = intervals;

    this.statuses = new ArrayList<Integer>();
    if (statuses != null) {
      for (int status : statuses)
        this.statuses.add(status);
    }
  }

  public Reschedule(String intervals, Collection<Integer> statuses) throws AdminException {
    vh.checkMaches("intervals", intervals, intervalsPattern);
    vh.checkNoNulls("statuses", statuses);

    this.intervals = intervals;
    this.statuses = new ArrayList<Integer>();
    if (statuses != null)
      this.statuses.addAll(statuses);
  }

  public Reschedule(Reschedule copy) {
    this.intervals = copy.intervals;
    this.statuses = new ArrayList<Integer>(copy.statuses);
  }

  public Collection<Integer> getStatuses() {
    return new ArrayList<Integer>(statuses);
  }

  public String getIntervals() {
    return intervals;
  }

  public boolean equals(Object o) {
    if (o instanceof Reschedule) {
      Reschedule r = (Reschedule) o;

      return r.intervals.equals(intervals) && r.statuses.containsAll(statuses) && statuses.containsAll(r.statuses);
    }
    return false;
  }

}
