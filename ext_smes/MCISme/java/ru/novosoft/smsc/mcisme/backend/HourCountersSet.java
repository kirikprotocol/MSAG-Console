package ru.novosoft.smsc.mcisme.backend;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 18:03:51
 * To change this template use Options | File Templates.
 */
public class HourCountersSet extends CountersSet
{
  private int hour = 0;

  HourCountersSet(int hour) {
    this.hour = hour;
  }

  HourCountersSet(long m, long d, long f, long n, int hour) {
    super(m, d, f, n); this.hour = hour;
  }

  HourCountersSet(CountersSet set, int hour) {
    super(set); this.hour = hour;
  }

  public int getHour() {
    return hour;
  }

}
