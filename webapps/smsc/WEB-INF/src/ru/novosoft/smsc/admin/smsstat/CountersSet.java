package ru.novosoft.smsc.admin.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:37:00 PM
 * To change this template use Options | File Templates.
 */

public class CountersSet
{
  public long accepted = 0;
  public long rejected = 0;
  public long delivered = 0;
  public long failed = 0;
  public long rescheduled = 0;
  public long temporal = 0;
  public long peak_i = 0;
  public long peak_o = 0;

  public CountersSet()
  {
  }

  public CountersSet(long accepted, long rejected, long delivered,
                     long failed, long rescheduled, long temporal,
                     long i, long o)
  {
    this.accepted = accepted;
    this.rejected = rejected;
    this.delivered = delivered;
    this.failed = failed;
    this.rescheduled = rescheduled;
    this.temporal = temporal;
    this.peak_i = i;
    this.peak_o = o;
  }

  protected void increment(CountersSet set)
  {
    this.accepted += set.accepted;
    this.rejected += set.rejected;
    this.delivered += set.delivered;
    this.failed += set.failed;
    this.rescheduled += set.rescheduled;
    this.temporal += set.temporal;
    if (set.peak_i > this.peak_i) this.peak_i = set.peak_i;
    if (set.peak_o > this.peak_o) this.peak_o = set.peak_o;
  }

}
