/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:38:40 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

public class SmeIdCountersSet extends ExtendedCountersSet
{
  public String smeid;

  public SmeIdCountersSet(String smeid)
  {
    this.smeid = smeid;
  }

  public SmeIdCountersSet(int accepted, int rejected, int delivered,
                          int failed, int rescheduled, int temporal, String smeid)
  {
    super(accepted, rejected, delivered, failed, rescheduled, temporal);
    this.smeid = smeid;
  }

}
