package ru.novosoft.smsc.infosme.backend;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.09.2003
 * Time: 13:26:37
 * To change this template use Options | File Templates.
 */

public class HourCountersSet extends CountersSet implements Comparable
{
  private int hour = 0;

  HourCountersSet(int hour)
  {
    this.hour = hour;
  }

  public HourCountersSet(long g, long d, long r, long f, int hour)
  {
    super(g, d, r, f);
    this.hour = hour;
  }

  HourCountersSet(CountersSet set, int hour)
  {
    super(set);
    this.hour = hour;
  }

  public int getHour()
  {
    return hour;
  }

  public int compareTo(Object o) {
    int h = ((HourCountersSet)o).hour;
    if ( h > hour)
        return -1;
    return h == hour ? 0 : 1;
  }
}
