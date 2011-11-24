package ru.novosoft.smsc.admin.mcisme;

/**
 * author: Aleksandr Khalitov
 */
public class Statistics {

  private int missed;
  private int delivered;
  private int failed;
  private int notified;

  public int getMissed() {
    return missed;
  }

  public void setMissed(int missed) {
    this.missed = missed;
  }

  public int getDelivered() {
    return delivered;
  }

  public void setDelivered(int delivered) {
    this.delivered = delivered;
  }

  public int getFailed() {
    return failed;
  }

  public void setFailed(int failed) {
    this.failed = failed;
  }

  public int getNotified() {
    return notified;
  }

  public void setNotified(int notified) {
    this.notified = notified;
  }
}
