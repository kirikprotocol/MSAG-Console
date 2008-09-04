package ru.novosoft.smsc.emailsme.beans;

import java.util.*;

/**
 * User: artem
 * Date: 04.09.2008
 */

public class Statistic {

  private final Date date;

  private final Map stats = new HashMap(24);

  private int totalReceivedOk;
  private int totalReceivedFail;
  private int totalTransmittedOk;
  private int totalTransmittedFail;

  public Statistic(Date date) {
    this.date = date;
  }

  public Date getDate() {
    return date;
  }

  public int getTotalReceivedFail() {
    return totalReceivedFail;
  }

  public int getTotalReceivedOk() {
    return totalReceivedOk;
  }

  public int getTotalTransmittedFail() {
    return totalTransmittedFail;
  }

  public int getTotalTransmittedOk() {
    return totalTransmittedOk;
  }

  public void addStat(int hour, int receivedOk, int receivedFail, int transmitedOk, int transmitedFail) {
    Integer key = new Integer(hour);
    StatRecord r = (StatRecord)stats.get(key);
    if (r == null) {
      r = new StatRecord(hour);
      stats.put(key, r);
    }
    r.receivedOk += receivedOk;
    r.receivedFail += receivedFail;
    r.transmittedOk += transmitedOk;
    r.transmittedFail += transmitedFail;
    totalReceivedOk += receivedOk;
    totalReceivedFail += receivedFail;
    totalTransmittedFail += transmitedFail;
    totalTransmittedOk += transmitedOk;
  }

  public Collection getStats() {
    return new TreeSet(stats.values());
  }

  public static class StatRecord implements Comparable {
    private final int hour;
    private int receivedOk;
    private int receivedFail;
    private int transmittedOk;
    private int transmittedFail;

    public StatRecord(int hour) {
      this.hour = hour;
    }

    public int getHour() {
      return hour;
    }

    public int getReceivedFail() {
      return receivedFail;
    }

    public int getReceivedOk() {
      return receivedOk;
    }

    public int getTransmittedFail() {
      return transmittedFail;
    }

    public int getTransmittedOk() {
      return transmittedOk;
    }

    public int compareTo(Object o) {
      StatRecord r = (StatRecord)o;
      if (hour < r.hour)
        return -1;
      else if (hour > r.hour)
        return 1;
      return 0;
    }
  }
}
