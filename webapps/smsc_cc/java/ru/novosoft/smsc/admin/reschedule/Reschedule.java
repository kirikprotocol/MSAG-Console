package ru.novosoft.smsc.admin.reschedule;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class Reschedule {

  private final Collection<Integer> statuses;
  private final String intervals;

  public Reschedule(String intervals, int... statuses) {
    if (intervals == null)
      throw new IllegalArgumentException("Intervals are null");

    this.intervals = intervals;
    this.statuses = new ArrayList<Integer>();
    if (statuses != null) {
      for (int status : statuses)
        this.statuses.add(status);
    }
  }

  public Reschedule(String intervals, Collection<Integer> statuses) {
    if (intervals == null)
      throw new IllegalArgumentException("Intervals are null");

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
