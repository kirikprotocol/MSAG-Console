package ru.sibinco.smppgw.backend.stat.stat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 13:51:48
 * To change this template use File | Settings | File Templates.
 */
public class CountersSet
{
  protected long accepted = 0;
  protected long rejected = 0;
  protected long delivered = 0;
  protected long tempError = 0;
  protected long permError = 0;

  protected long smsTrOk = 0;
  protected long smsTrFailed = 0;
  protected long smsTrBilled = 0;
  protected long ussdTrFromScOk = 0;
  protected long ussdTrFromScFailed = 0;
  protected long ussdTrFromScBilled = 0;
  protected long ussdTrFromSmeOk = 0;
  protected long ussdTrFromSmeFailed = 0;
  protected long ussdTrFromSmeBilled = 0;

  public CountersSet() {}
  public CountersSet(long accepted, long rejected, long delivered,
                     long tempError, long permError)
  {
    this.accepted = accepted;  this.rejected = rejected; this.delivered = delivered;
    this.tempError = tempError; this.permError = permError;
  }

  public CountersSet(long accepted, long rejected, long delivered, long tempError, long permError,
                     long smsTrOk, long smsTrFailed, long smsTrBilled,
                     long ussdTrFromScOk, long ussdTrFromScFailed, long ussdTrFromScBilled,
                     long ussdTrFromSmeOk, long ussdTrFromSmeFailed, long ussdTrFromSmeBilled)
  {
    this.accepted = accepted; this.rejected = rejected; this.delivered = delivered;
    this.tempError = tempError; this.permError = permError;
    this.smsTrOk = smsTrOk; this.smsTrFailed = smsTrFailed; this.smsTrBilled = smsTrBilled;
    this.ussdTrFromScOk = ussdTrFromScOk; this.ussdTrFromScFailed = ussdTrFromScFailed;
    this.ussdTrFromScBilled = ussdTrFromScBilled; this.ussdTrFromSmeOk = ussdTrFromSmeOk;
    this.ussdTrFromSmeFailed = ussdTrFromSmeFailed; this.ussdTrFromSmeBilled = ussdTrFromSmeBilled;
  }

  protected void increment(CountersSet set)
  {
    this.accepted += set.accepted; this.rejected += set.rejected; this.delivered += set.delivered;
    this.tempError += set.tempError; this.permError += set.permError;
    this.smsTrOk += set.smsTrOk; this.smsTrFailed += set.smsTrFailed; this.smsTrBilled += set.smsTrBilled;
    this.ussdTrFromScOk += set.ussdTrFromScOk; this.ussdTrFromScFailed += set.ussdTrFromScFailed;
    this.ussdTrFromScBilled += set.ussdTrFromScBilled; this.ussdTrFromSmeOk += set.ussdTrFromSmeOk;
    this.ussdTrFromSmeFailed += set.ussdTrFromSmeFailed; this.ussdTrFromSmeBilled += set.ussdTrFromSmeBilled;
  }

  public long getAccepted() {
    return accepted;
  }
  public long getRejected() {
    return rejected;
  }
  public long getDelivered() {
    return delivered;
  }
  public long getTempError() {
    return tempError;
  }
  public long getPermError() {
    return permError;
  }
  public long getSmsTrOk() {
    return smsTrOk;
  }
  public long getSmsTrFailed() {
    return smsTrFailed;
  }
  public long getSmsTrBilled() {
    return smsTrOk;
  }
  public long getUssdTrFromScOk() {
    return ussdTrFromScOk;
  }
  public long getUssdTrFromScFailed() {
    return ussdTrFromScFailed;
  }
  public long getUssdTrFromScBilled() {
    return ussdTrFromScBilled;
  }
  public long getUssdTrFromSmeOk() {
    return ussdTrFromSmeOk;
  }
  public long getUssdTrFromSmeFailed() {
    return ussdTrFromSmeFailed;
  }
  public long getUssdTrFromSmeBilled() {
    return ussdTrFromSmeBilled;
  }

}
