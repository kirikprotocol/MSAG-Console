package ru.sibinco.otasme.network;

import java.util.Comparator;

public class RetryTask implements Comparator {

  private OutgoingObject outgoingObject;
  private long retryTime = -1;

  public RetryTask() {
  }

  public RetryTask(OutgoingObject outgoingObject) {
    this.outgoingObject = outgoingObject;
    this.retryTime = System.currentTimeMillis() + outgoingObject.getRetryPeriod();
  }

  public OutgoingObject getOutgoingObject() {
    return outgoingObject;
  }

  public long getRetryTime() {
    return retryTime;
  }

  public int getRetries() {
    return outgoingObject.getRetries();
  }

  public boolean equals(Object o) {
    if (this == o) return true;
    if (!(o instanceof RetryTask)) return false;

    final RetryTask retryTask = (RetryTask) o;

    if (retryTime != retryTask.retryTime) return false;

    return true;
  }

  public int compare(Object obj1, Object obj2) {
    if (obj1 instanceof RetryTask && obj2 instanceof RetryTask) {
      return (int) (((RetryTask) obj1).getRetryTime() - ((RetryTask) obj2).getRetryTime());
    } else {
      return 0;
    }
  }
}
