package ru.novosoft.smsc.admin.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:38:40 PM
 * To change this template use Options | File Templates.
 */


public class SmeIdCountersSet extends ExtendedCountersSet implements Comparable
{
  public String smeid;

  public SmeIdCountersSet(String smeid)
  {
    this.smeid = smeid;
  }

  public SmeIdCountersSet(long accepted, long rejected, long delivered,
                          long failed, long rescheduled, long temporal, long i, long o, String smeid)
  {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    this.smeid = smeid;
  }

  public int compareTo(Object o)
  {
    if (o == null || smeid == null || !(o instanceof SmeIdCountersSet)) return -1;
    return smeid.compareTo(((SmeIdCountersSet) o).smeid);
  }
}
