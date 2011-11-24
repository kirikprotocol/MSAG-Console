package ru.novosoft.smsc.admin.mcisme;

/**
 * author: Aleksandr Khalitov
 */
public class CountersSet {
  private long missed;
  private long delivered;
  private long failed;
  private long notified;

  public CountersSet() {
  }

  public CountersSet(CountersSet set) {
    missed = set.missed;
    delivered = set.delivered;
    failed = set.failed;
    notified = set.notified;
  }

  public CountersSet(long m, long d, long f, long n) {
    missed = m;
    delivered = d;
    failed = f;
    notified = n;
  }

  public void increment(CountersSet set) {
    missed += set.missed;
    delivered += set.delivered;
    failed += set.failed;
    notified += set.notified;
  }

  public void reset() {
    missed = 0;
    delivered = 0;
    failed = 0;
    notified = 0;
  }

  public long getMissed() {
    return missed;
  }

  public void setMissed(long missed) {
    this.missed = missed;
  }

  public long getDelivered() {
    return delivered;
  }

  public void setDelivered(long delivered) {
    this.delivered = delivered;
  }

  public long getFailed() {
    return failed;
  }

  public void setFailed(long failed) {
    this.failed = failed;
  }

  public long getNotified() {
    return notified;
  }

  public void setNotified(long notified) {
    this.notified = notified;
  }
}
