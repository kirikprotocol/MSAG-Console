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
  public HourCountersSet(long accepted, long rejected, long delivered,
                         long tempError, long permError, int hour)
  {
    super(accepted, rejected, delivered, tempError, permError);
    this.hour = hour;
  }
  public HourCountersSet(long accepted, long rejected, long delivered, long tempError, long permError,
                         long smsTrOk, long smsTrFailed, long smsTrBilled,
                         long ussdTrFromScOk, long ussdTrFromScFailed, long ussdTrFromScBilled,
                         long ussdTrFromSmeOk, long ussdTrFromSmeFailed, long ussdTrFromSmeBilled, int hour)
  {
    super(accepted, rejected, delivered, tempError, permError,
          smsTrOk, smsTrFailed, smsTrBilled, 
          ussdTrFromScOk, ussdTrFromScFailed, ussdTrFromScBilled,
          ussdTrFromSmeOk, ussdTrFromSmeFailed, ussdTrFromSmeBilled);
    this.hour = hour;
  }

  public int getHour() {
    return hour;
  }
}
