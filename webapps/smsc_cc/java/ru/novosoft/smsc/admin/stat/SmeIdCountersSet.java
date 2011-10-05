package ru.novosoft.smsc.admin.stat;

/**
 * author: Aleksandr Khalitov
 */
public class SmeIdCountersSet extends ExtendedCountersSet implements Comparable {

  private String smeid;

  SmeIdCountersSet(String smeid){
    this.smeid = smeid;
  }

  SmeIdCountersSet(long accepted, long rejected, long delivered,
                          long failed, long rescheduled, long temporal, long i, long o, String smeid) {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    this.smeid = smeid;
  }

  public int compareTo(Object o) {
    if (o == null || smeid == null || !(o instanceof SmeIdCountersSet)) return -1;
    return smeid.compareTo(((SmeIdCountersSet) o).smeid);
  }

  public String getSmeid() {
    return smeid;
  }
}
