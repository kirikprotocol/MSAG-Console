package ru.novosoft.smsc.admin.mcisme;

/**
 * author: Aleksandr Khalitov
 */
public class RunStatistics {

  private int activeTasks;
  private int inQueueSize;
  private int outQueueSize;
  private int inSpeed;
  private int outSpeed;

  public int getActiveTasks() {
    return activeTasks;
  }

  public void setActiveTasks(int activeTasks) {
    this.activeTasks = activeTasks;
  }

  public int getInQueueSize() {
    return inQueueSize;
  }

  public void setInQueueSize(int inQueueSize) {
    this.inQueueSize = inQueueSize;
  }

  public int getOutQueueSize() {
    return outQueueSize;
  }

  public void setOutQueueSize(int outQueueSize) {
    this.outQueueSize = outQueueSize;
  }

  public int getInSpeed() {
    return inSpeed;
  }

  public void setInSpeed(int inSpeed) {
    this.inSpeed = inSpeed;
  }

  public int getOutSpeed() {
    return outSpeed;
  }

  public void setOutSpeed(int outSpeed) {
    this.outSpeed = outSpeed;
  }
}
