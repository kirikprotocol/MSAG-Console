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
  public int providerId = -1;
   public SmeIdCountersSet(String smeid)
  {
    this.smeid = smeid;
  }

  public SmeIdCountersSet(long accepted, long rejected, long delivered, long tempError, long permError, String smeid)
  {
    super(accepted, rejected, delivered, tempError, permError);
    this.smeid = smeid;
  }

  public SmeIdCountersSet(long accepted, long rejected, long delivered, long tempError, long permError,
                          long smsTrOk, long smsTrFailed, long smsTrBilled,
                          long ussdTrFromScOk, long ussdTrFromScFailed, long ussdTrFromScBilled,
                          long ussdTrFromSmeOk, long ussdTrFromSmeFailed, long ussdTrFromSmeBilled,
                          String smeid)
  {
    super(accepted, rejected, delivered, tempError, permError,
          smsTrOk, smsTrFailed, smsTrBilled,
          ussdTrFromScOk, ussdTrFromScFailed, ussdTrFromScBilled,
          ussdTrFromSmeOk, ussdTrFromSmeFailed, ussdTrFromSmeBilled);
    this.smeid = smeid;
  }

  public int compareTo(Object o) {
    if (o == null || smeid == null || !(o instanceof SmeIdCountersSet)) return -1;
    return smeid.compareTo(((SmeIdCountersSet)o).smeid);
  }

  public String getSmeid() {
    return smeid;
  }

  public int getProviderId()
  {
    return providerId;
  }

  public void setProviderId(int providerId)
  {
    this.providerId = providerId;
  }
}
