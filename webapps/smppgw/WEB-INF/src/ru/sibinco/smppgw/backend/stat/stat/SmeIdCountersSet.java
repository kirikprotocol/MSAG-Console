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

  protected int smsTrOk = 0;
  protected int smsTrFailed = 0;
  protected int ussdTrFromScOk = 0;
  protected int ussdTrFromScFailed = 0;
  protected int ussdTrFromScBilled = 0;
  protected int ussdTrFromSmeOk = 0;
  protected int ussdTrFromSmeFailed = 0;
  protected int ussdTrFromSmeBilled = 0;

  public SmeIdCountersSet(int accepted, int rejected, int delivered, int tempError, int permError,
                          int smsTrOk, int smsTrFailed, int ussdTrFromScOk, int ussdTrFromScFailed,
                          int ussdTrFromScBilled, int ussdTrFromSmeOk, int ussdTrFromSmeFailed,
                          int ussdTrFromSmeBilled, String smeid)
  {
    super(accepted, rejected, delivered, tempError, permError);
    this.smeid = smeid;
    this.smsTrOk = smsTrOk;
    this.smsTrFailed = smsTrFailed;
    this.ussdTrFromScOk = ussdTrFromScOk;
    this.ussdTrFromScFailed = ussdTrFromScFailed;
    this.ussdTrFromScBilled = ussdTrFromScBilled;
    this.ussdTrFromSmeOk = ussdTrFromSmeOk;
    this.ussdTrFromSmeFailed = ussdTrFromSmeFailed;
    this.ussdTrFromSmeBilled = ussdTrFromSmeBilled;
  }

  public int compareTo(Object o) {
    if (o == null || smeid == null || !(o instanceof SmeIdCountersSet)) return -1;
    return smeid.compareTo(((SmeIdCountersSet)o).smeid);
  }

  public String getSmeid() {
    return smeid;
  }

  public int getSmsTrOk() {
    return smsTrOk;
  }
  public int getSmsTrFailed() {
    return smsTrFailed;
  }
  public int getUssdTrFromScOk() {
    return ussdTrFromScOk;
  }
  public int getUssdTrFromScFailed() {
    return ussdTrFromScFailed;
  }
  public int getUssdTrFromScBilled() {
    return ussdTrFromScBilled;
  }
  public int getUssdTrFromSmeOk() {
    return ussdTrFromSmeOk;
  }
  public int getUssdTrFromSmeFailed() {
    return ussdTrFromSmeFailed;
  }
  public int getUssdTrFromSmeBilled() {
    return ussdTrFromSmeBilled;
  }
}
