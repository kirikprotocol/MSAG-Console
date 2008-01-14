package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;

public class OutgoingObject {
  private int id = -1;
  private String messageStatusListenerName = null;
  private Message outgoingMessage = null;
  private int retries = -1;
  private long sendTime = -1;
  private long retryPeriod = -1;
  private String connector = null;


  public OutgoingObject(Message messageData) {
    setOutgoingMessage(messageData);
  }

  public Message getOutgoingMessage() {
    return outgoingMessage;
  }

  protected void setOutgoingMessage(Message messageData) {
    //id = messageData.getId();
    outgoingMessage = messageData;
    connector = messageData.getConnectionName();
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

  public String getConnector() {
    return connector;
  }

  public void send() {
    retries++;
    sendTime = System.currentTimeMillis();
  }

  protected String getMessageStatusListenerName() {
    return messageStatusListenerName;
  }

  protected int getId() {
    return id;
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
