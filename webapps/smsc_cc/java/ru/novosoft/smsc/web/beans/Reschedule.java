package ru.novosoft.smsc.web.beans;

import java.io.Serializable;
import java.util.LinkedList;
import java.util.List;

/**
 * author: alkhal
 *
 **/

public class Reschedule implements Serializable{

  private String intervals;

  private List<Status> statuses = new LinkedList<Status>();

  private boolean checked;

  public Reschedule() {
  }
  public Reschedule(String intervals) {
    this.intervals = intervals;
  }

  public Reschedule(String intervals, List<Status> statuses) {
    this.intervals = intervals;
    this.statuses = statuses;
  }

  public boolean isChecked() {
    return checked;
  }

  public void setChecked(boolean checked) {
    this.checked = checked;
  }
  public String getIntervals() {
    return intervals;
  }

  public void setIntervals(String intervals) {
    this.intervals = intervals;
  }

  public List<Status> getStatuses() {
    return statuses;
  }

  public void setStatuses(List<Status> statuses) {
    this.statuses = statuses;
  }

  public void addStatus(Status status) {
    statuses.add(status);
  }

  public void removeStatus(int i) {
    statuses.remove(i);
  }

  public static class Status implements Serializable {
    private Integer status;

    public Status() {
    }

    public Status(Integer status) {
      this.status = status;
    }

    public Integer getStatus() {
      return status;
    }

    public void setStatus(Integer status) {
      this.status = status;
    }

    @Override
    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;

      Status status1 = (Status) o;

      if (status != null ? !status.equals(status1.status) : status1.status != null) return false;

      return true;
    }

    @Override
    public int hashCode() {
      return status != null ? status.hashCode() : 0;
    }
  }

}
