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

  protected int SmsTrOk = 0;
  protected int SmsTrFailed = 0;
  protected int UssdTrFromScOk = 0;
  protected int UssdTrFromScFailed = 0;
  protected int UssdTrFromScBilled = 0;
  protected int UssdTrFromSmeOk = 0;
  protected int UssdTrFromSmeFailed = 0;
  protected int UssdTrFromSmeBilled = 0;

  public SmeIdCountersSet(int accepted, int rejected, int delivered, int tempError, int permError,
                          int smsTrOk, int smsTrFailed, int ussdTrFromScOk, int ussdTrFromScFailed,
                          int ussdTrFromScBilled, int ussdTrFromSmeOk, int ussdTrFromSmeFailed,
                          int ussdTrFromSmeBilled, String smeid)
  {
    super(accepted, rejected, delivered, tempError, permError);
    this.smeid = smeid;
    SmsTrOk = smsTrOk;
    SmsTrFailed = smsTrFailed;
    UssdTrFromScOk = ussdTrFromScOk;
    UssdTrFromScFailed = ussdTrFromScFailed;
    UssdTrFromScBilled = ussdTrFromScBilled;
    UssdTrFromSmeOk = ussdTrFromSmeOk;
    UssdTrFromSmeFailed = ussdTrFromSmeFailed;
    UssdTrFromSmeBilled = ussdTrFromSmeBilled;
  }

  public int compareTo(Object o) {
    if (o == null || smeid == null || !(o instanceof SmeIdCountersSet)) return -1;
    return smeid.compareTo(((SmeIdCountersSet)o).smeid);
  }

  public String getSmeid() {
    return smeid;
  }

  public int getSmsTrOk() {
    return SmsTrOk;
  }
  public int getSmsTrFailed() {
    return SmsTrFailed;
  }
  public int getUssdTrFromScOk() {
    return UssdTrFromScOk;
  }
  public int getUssdTrFromScFailed() {
    return UssdTrFromScFailed;
  }
  public int getUssdTrFromScBilled() {
    return UssdTrFromScBilled;
  }
  public int getUssdTrFromSmeOk() {
    return UssdTrFromSmeOk;
  }
  public int getUssdTrFromSmeFailed() {
    return UssdTrFromSmeFailed;
  }
  public int getUssdTrFromSmeBilled() {
    return UssdTrFromSmeBilled;
  }
}
