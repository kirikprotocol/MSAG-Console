package ru.sibinco.otasme.network;

import ru.aurorisoft.smpp.Message;

import java.util.LinkedList;
import java.util.List;

public class OutgoingObject {
  private Message outgoingMessage = null;
  private List additionalObjects = new LinkedList();
  private Message incominMessage = null;
  private int retries = -1;
  private long sendTime = -1;
  private long retryPeriod = -1;

  public OutgoingObject(Message outgoingMessage) {
    this.outgoingMessage = outgoingMessage;
  }

  public Message getOutgoingMessage() {
    return outgoingMessage;
  }

  public Message getIncominMessage() {
    return incominMessage;
  }

  public OutgoingObject getAdditionalObject(int index) {
    return (OutgoingObject) additionalObjects.get(index);
  }

  public List getAdditionalObjects() {
    return additionalObjects;
  }

  public void addAdditionalObject(OutgoingObject additionalObject) {
    additionalObjects.add(additionalObject);
  }

  public void addAdditionalObjects(List additionalObjects) {
    this.additionalObjects.addAll(additionalObjects);
  }

  public void setIncominMessage(Message incominMessage) {
    this.incominMessage = incominMessage;
  }

  public int getRetries() {
    return retries;
  }

  public long getSendTime() {
    return sendTime;
  }

  public long getRetryPeriod() {
    return retryPeriod;
  }

  public void setRetryPeriod(long retryPeriod) {
    this.retryPeriod = retryPeriod;
  }

  public void send() {
    retries++;
    sendTime = System.currentTimeMillis();
  }

  public boolean equals(Object obj) {
    if (obj instanceof Long)
      return new Long(((long) outgoingMessage.getConnectionId()) << 32 | outgoingMessage.getSequenceNumber()).longValue() == ((Long) obj).longValue();
    else
      return false;
  }

  public int hashCode() {
    return new Long(((long) outgoingMessage.getConnectionId()) << 32 | outgoingMessage.getSequenceNumber()).hashCode();
  }
}
