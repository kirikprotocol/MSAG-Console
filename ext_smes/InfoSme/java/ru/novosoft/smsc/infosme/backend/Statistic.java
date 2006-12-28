package ru.novosoft.smsc.infosme.backend;

import java.util.Date;

/**
 * User: artem
 * Date: 29.11.2006
 */
public class Statistic {

  private Date period = null;
  private long generated = 0;
  private long delivered = 0;
  private long retried = 0;
  private long failed = 0;

  public long getGenerated() {
    return generated;
  }

  public void setGenerated(long generated) {
    this.generated = generated;
  }

  public long getDelivered() {
    return delivered;
  }

  public void setDelivered(long delivered) {
    this.delivered = delivered;
  }

  public long getRetried() {
    return retried;
  }

  public void setRetried(long retried) {
    this.retried = retried;
  }

  public long getFailed() {
    return failed;
  }

  public void setFailed(long failed) {
    this.failed = failed;
  }

  public Date getPeriod() {
    return period;
  }

  public void setPeriod(Date period) {
    this.period = period;
  }
}
