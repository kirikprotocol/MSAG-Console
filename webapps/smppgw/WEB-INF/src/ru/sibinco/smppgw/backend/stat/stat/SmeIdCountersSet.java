package ru.sibinco.smppgw.backend.stat.stat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 14:02:01
 * To change this template use File | Settings | File Templates.
 */
public class SmeIdCountersSet extends ExtendedCountersSet implements Comparable
{
  protected String smeid;
  // TODO: add transactional counters

  public SmeIdCountersSet(String smeid) {
    this.smeid = smeid;
  }

  public SmeIdCountersSet(int accepted, int rejected, int delivered,
                          int tempError, int permError, String smeid)
  {
    super(accepted, rejected, delivered, tempError, permError);
    this.smeid = smeid;
  }

  public int compareTo(Object o) {
    if (o == null || smeid == null || !(o instanceof SmeIdCountersSet)) return -1;
    return smeid.compareTo(((SmeIdCountersSet)o).smeid);
  }

  public String getSmeid() {
    return smeid;
  }
}
