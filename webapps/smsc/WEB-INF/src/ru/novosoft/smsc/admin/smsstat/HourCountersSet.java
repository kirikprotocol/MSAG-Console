/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 11, 2002
 * Time: 1:31:30 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

public class HourCountersSet extends CountersSet
{
  private int hour = 0;

  public HourCountersSet(int hour) {
    this.hour = hour;
  }
  public HourCountersSet(int accepted, int rejected, int delivered, int failed,
                         int rescheduled, int temporal, int i, int o, int hour)
  {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    this.hour = hour;
  }
  public int getHour() {
    return hour;
  }
}
