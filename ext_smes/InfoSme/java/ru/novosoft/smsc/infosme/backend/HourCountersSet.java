package ru.novosoft.smsc.infosme.backend;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.09.2003
 * Time: 13:26:37
 * To change this template use Options | File Templates.
 */

public class HourCountersSet extends CountersSet
{
  private int hour = 0;

  HourCountersSet(int hour)
  {
    this.hour = hour;
  }

  HourCountersSet(long g, long d, long r, long f, int hour)
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
}
