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
  // TODO: possible use 'long' instead of 'int'

  public int accepted = 0;
  public int rejected = 0;
  public int delivered = 0;
  public int tempError = 0;
  public int permError = 0;

  public CountersSet() {}
  public CountersSet(int accepted, int rejected, int delivered,
                     int tempError, int permError)
  {
    this.accepted = accepted;  this.rejected = rejected; this.delivered = delivered;
    this.tempError = tempError; this.permError = permError;
  }

  protected void increment(CountersSet set) {
    this.accepted += set.accepted;  this.rejected += set.rejected;
    this.delivered += set.delivered;
    this.tempError += set.tempError;
    this.permError += set.permError;
  }

  public int getAccepted() {
    return accepted;
  }
  public int getRejected() {
    return rejected;
  }
  public int getDelivered() {
    return delivered;
  }
  public int getTempError() {
    return tempError;
  }
  public int getPermError() {
    return permError;
  }
}
