package ru.sibinco.smsx.network;

import ru.aurorisoft.smpp.Message;

public class OutgoingObject {

  public static int DELIVER_OK = 0;
  public static int DELIVER_TMP_ERROR = 1;
  public static int DELIVER_THROTTLED_ERROR = 2;
  public static int DELIVER_PERM_ERROR = 3;
  public static int DELIVER_UNKNOWN_ERROR = 4;

  private Message outgoingMessage = null;
  private int retries = -1;
  private long sendTime = -1;
  private long retryPeriod = -1;
  private int id = -1;

  public OutgoingObject(Message outgoingMessage) {
    this.outgoingMessage = outgoingMessage;
  }

  public Message getOutgoingMessage() {
    return outgoingMessage;
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

  public void changeStatus(int newStatus) {
  }

  public int getId() {
    return id;
  }

  public void setId(int id) {
    this.id = id;
  }
}
