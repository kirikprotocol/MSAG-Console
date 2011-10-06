package ru.novosoft.smsc.admin.stat;

/**
 * author: Aleksandr Khalitov
 */
public class CountersSet {

  private long accepted = 0;
  private long rejected = 0;
  private long delivered = 0;
  private long failed = 0;
  private long rescheduled = 0;
  private long temporal = 0;
  private long peak_i = 0;
  private long peak_o = 0;

  CountersSet() {
  }

  CountersSet(long accepted, long rejected, long delivered,
                     long failed, long rescheduled, long temporal,
                     long i, long o){
    this.increment(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
  }

  protected void increment(CountersSet set) {
    this.accepted += set.accepted; this.rejected += set.rejected;
    this.delivered += set.delivered; this.failed += set.failed;
    this.rescheduled += set.rescheduled; this.temporal += set.temporal;
    if (set.peak_i > this.peak_i) this.peak_i = set.peak_i;
    if (set.peak_o > this.peak_o) this.peak_o = set.peak_o;
  }

  protected void increment(long accepted, long rejected, long delivered,
                           long failed, long rescheduled, long temporal,
                           long i, long o) {
    this.accepted += accepted; this.rejected += rejected;
    this.delivered += delivered; this.failed += failed;
    this.rescheduled += rescheduled; this.temporal += temporal;
    if (i > this.peak_i) this.peak_i = i;
    if (o > this.peak_o) this.peak_o = o;
  }

  void reset(){
    accepted = 0; rejected = 0; delivered = 0; failed = 0;
    rescheduled = 0; temporal = 0; peak_i = 0; peak_o = 0;
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

  public long getFailed() {
    return failed;
  }

  public long getRescheduled() {
    return rescheduled;
  }

  public long getTemporal() {
    return temporal;
  }

  public long getPeak_i() {
    return peak_i;
  }

  public long getPeak_o() {
    return peak_o;
  }
}
