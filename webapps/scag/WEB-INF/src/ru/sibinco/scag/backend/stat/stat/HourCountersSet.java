package ru.sibinco.scag.backend.stat.stat;

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

  public HourCountersSet(int hour)
  {
    this.hour = hour;
  }
  public HourCountersSet(long accepted, long rejected,
                     long gw_rejected, long delivered, long failed,
                     long recieptOk, long recieptFailed, int hour)
  {
    super(accepted, rejected, gw_rejected, delivered, failed, recieptOk, recieptFailed);
    this.hour = hour;
  }

  public int getHour() {
    return hour;
  }
}
