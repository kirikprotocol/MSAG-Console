package ru.novosoft.smsc.infosme.backend.siebel;

import java.util.Date;

/**
 * author: alkhal
 */

public class SiebelMessage {
  private String clcId;  //unique
  private Date created;
  private Date lastUpd;
  private String waveId;
  private String msisdn;
  private String message;

  private State state;
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

  public State getMessageState() {
    return state;
  }

  public void setMessageState(State state) {
    this.state = state;
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

  public String toString() {
    return "SiebelMessage{" +
        "smscValue='" + smscValue + '\'' +
        ", smscCode='" + smscCode + '\'' +
        ", state=" + state +
        ", message='" + message + '\'' +
        ", msisdn='" + msisdn + '\'' +
        ", waveId='" + waveId + '\'' +
        ", lastUpd=" + lastUpd +
        ", created=" + created +
        ", clcId='" + clcId + '\'' +
        '}';
  }

  public static class State {

    public static State ENROUTE = new State("ENROUTE");
    public static State DELIVERED = new State("DELIVERED");
    public static State EXPIRED = new State("EXPIRED");
    public static State DELETED = new State("DELETED");
    public static State UNDELIVERABLE = new State("UNDELIVERABLE");
    public static State ACCEPTED = new State("ACCEPTED");
    public static State UNKNOWN = new State("UNKNOWN");
    public static State REJECTED = new State("REJECTED");
    public static State ERROR = new State("ERROR");


    private String value;
    private State(String value) {
      this.value = value;
    }

    public String toString() {
      return value;
    }

    public static State valueOf(String st) {
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

    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;

      State state = (State) o;

      if (value != null ? !value.equals(state.value) : state.value != null) return false;

      return true;
    }
  }

  public static class SmppState {
    private String code;
    private String value;

    public String getCode() {
      return code;
    }

    public String getValue() {
      return value;
    }

    public void setCode(String code) {
      this.code = code;
    }

    public void setValue(String value) {
      this.value = value;
    }

    public String toString() {
      return "SmppState{" +
          "code='" + code + '\'' +
          ", value='" + value + '\'' +
          '}';
    }
  }

  public static class DeliveryState {
    private final State state;
    private final SmppState smppState;

    public DeliveryState(State state, SmppState smppState) {
      this.state = state;
      this.smppState = smppState;
    }

    public State getState() {
      return state;
    }

    public SmppState getSmppState() {
      return smppState;
    }

    public String toString() {
      return "DeliveryState{" +
          "state=" + state +
          ", smppState=" + smppState +
          '}';
    }
  }
}
