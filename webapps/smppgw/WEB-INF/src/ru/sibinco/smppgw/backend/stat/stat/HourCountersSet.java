package ru.sibinco.smppgw.backend.stat.stat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 13:56:08
 * To change this template use File | Settings | File Templates.
 */
public class HourCountersSet extends CountersSet
{
  private int hour = 0;

  public HourCountersSet(int hour) {
    this.hour = hour;
  }
  public HourCountersSet(int accepted, int rejected, int delivered,
                        int tempError, int permError, int hour)
  {
    super(accepted, rejected, delivered, tempError, permError);
    this.hour = hour;
  }
  public int getHour() {
    return hour;
  }
}
