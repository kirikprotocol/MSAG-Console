/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:37:00 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

public class CountersSet
{
  public int accepted = 0;
  public int rejected = 0;
  public int delivered = 0;
  public int failed = 0;
  public int rescheduled = 0;
  public int temporal = 0;

  public CountersSet() {}
  public CountersSet(int accepted, int rejected, int delivered,
                     int failed, int rescheduled, int temporal)
  {
    this.accepted = accepted;  this.rejected = rejected;
    this.delivered = delivered;  this.failed = failed;
    this.rescheduled = rescheduled; this.temporal = temporal;
  }

  protected void increment(CountersSet set) {
    this.accepted += set.accepted;  this.rejected += set.rejected;
    this.delivered += set.delivered;  this.failed += set.failed;
    this.rescheduled += set.rescheduled; this.temporal += set.temporal;
  }

}
