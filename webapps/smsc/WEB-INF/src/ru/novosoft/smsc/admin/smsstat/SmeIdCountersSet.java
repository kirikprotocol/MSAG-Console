/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:38:40 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

public class SmeIdCountersSet extends ExtendedCountersSet implements Comparable
{
  public String smeid;

  public SmeIdCountersSet(String smeid)
  {
    this.smeid = smeid;
  }

  public SmeIdCountersSet(int accepted, int rejected, int delivered,
                          int failed, int rescheduled, int temporal, int i, int o, String smeid)
  {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    this.smeid = smeid;
  }

  public int compareTo(Object o) {
    if (o == null || smeid == null || !(o instanceof SmeIdCountersSet)) return -1;
    return smeid.compareTo(((SmeIdCountersSet)o).smeid);
  }
}
