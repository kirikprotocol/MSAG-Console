package ru.novosoft.smsc.infosme.beans.deliveries;

/**
 * User: artem
 * Date: 04.06.2008
 */
public class DeliveriesGenerationProgress {
  private int total;
  private int msgCount;
  private int progressPercent;

  public DeliveriesGenerationProgress(int total) {
    this.total = total;
  }

  public void incProgress() {
    msgCount++;
    progressPercent = msgCount * 100 / total;
  }

  public int getMsgCount() {
    return msgCount;
  }

  public double getProgressPercent() {
    return progressPercent;
  }
}
