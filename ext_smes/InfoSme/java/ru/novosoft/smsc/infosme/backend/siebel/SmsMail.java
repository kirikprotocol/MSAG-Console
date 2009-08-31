package ru.novosoft.smsc.infosme.backend.siebel;

import java.util.Date;

/**
 * author: alkhal
 */

public class SmsMail {
  private String clcId;  //unique
  private Date created;
  private Date lastUpd;
  private String waveId;
  private String msisdn;
  private String message;

  private MessageState messageState;
  private String smscCode;
  private String smscValue;

  public String getClcId() {
    return clcId;
  }

  public void setClcId(String clcId) {
    this.clcId = clcId;
  }

  public Date getCreated() {
    return created;
  }

  public void setCreated(Date created) {
    this.created = created;
  }

  public Date getLastUpd() {
    return lastUpd;
  }

  public void setLastUpd(Date lastUpd) {
    this.lastUpd = lastUpd;
  }

  public String getWaveId() {
    return waveId;
  }

  public void setWaveId(String waveId) {
    this.waveId = waveId;
  }

  public String getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(String msisdn) {
    this.msisdn = msisdn;
  }

  public String getMessage() {
    return message;
  }

  public void setMessage(String message) {
    this.message = message;
  }

  public MessageState getMessageState() {
    return messageState;
  }

  public void setMessageState(MessageState messageState) {
    this.messageState = messageState;
  }

  public String getSmscCode() {
    return smscCode;
  }

  public void setSmscCode(String smscCode) {
    this.smscCode = smscCode;
  }

  public String getSmscValue() {
    return smscValue;
  }

  public void setSmscValue(String smscValue) {
    this.smscValue = smscValue;
  }

  @Override
  public String toString() {
    return "SmsMail{" +
        "smscValue='" + smscValue + '\'' +
        ", smscCode='" + smscCode + '\'' +
        ", messageState=" + messageState +
        ", message='" + message + '\'' +
        ", msisdn='" + msisdn + '\'' +
        ", waveId='" + waveId + '\'' +
        ", lastUpd=" + lastUpd +
        ", created=" + created +
        ", clcId='" + clcId + '\'' +
        '}';
  }

  public static class MessageState {

    public static MessageState ENROUTE = new MessageState("ENROUTE");
    public static MessageState DELIVERED = new MessageState("DELIVERED");
    public static MessageState EXPIRED = new MessageState("EXPIRED");
    public static MessageState DELETED = new MessageState("DELETED");
    public static MessageState UNDELIVERABLE = new MessageState("UNDELIVERABLE");
    public static MessageState ACCEPTED = new MessageState("ACCEPTED");
    public static MessageState UNKNOWN = new MessageState("UNKNOWN");
    public static MessageState REJECTED = new MessageState("REJECTED");
    public static MessageState ERROR = new MessageState("ERROR");


    private String value;
    private MessageState(String value) {
      this.value = value;
    }

    @Override
    public String toString() {
      return value;
    }

    public static MessageState valueOf(String st) {
      if(ENROUTE.value.equals(st)) {
        return ENROUTE;
      } else if(DELIVERED.value.equals(st)) {
        return DELIVERED;
      } else if(EXPIRED.value.equals(st)) {
        return EXPIRED;
      } else if(DELETED.value.equals(st)) {
        return DELETED;
      } else if(UNDELIVERABLE.value.equals(st)) {
        return UNDELIVERABLE;
      } else if(ACCEPTED.value.equals(st)) {
        return ACCEPTED;
      } else if(UNKNOWN.value.equals(st)) {
        return UNKNOWN;
      } else if(REJECTED.value.equals(st)) {
        return REJECTED;
      } else if(ERROR.value.equals(st)) {
        return ERROR;
      } else {
        throw new IllegalArgumentException("Unknown state: "+st);
      }
    }

    @Override
    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;

      MessageState state = (MessageState) o;

      if (value != null ? !value.equals(state.value) : state.value != null) return false;

      return true;
    }
  }

}
