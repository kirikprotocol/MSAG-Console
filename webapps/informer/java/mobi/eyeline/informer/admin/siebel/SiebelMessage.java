package mobi.eyeline.informer.admin.siebel;

import java.util.Date;

/**
 * Сообщение Siebel
 *
 * @author Aleksandr Khalitov
 */
class SiebelMessage {

  private String clcId;  //unique
  private Date created;
  private Date lastUpd;
  private String waveId;
  private String msisdn;
  private String message;
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
        ", message='" + message + '\'' +
        ", msisdn='" + msisdn + '\'' +
        ", waveId='" + waveId + '\'' +
        ", lastUpd=" + lastUpd +
        ", created=" + created +
        ", clcId='" + clcId + '\'' +
        '}';
  }

  public static enum State {
    ENROUTE,
    DELIVERED,
    EXPIRED,
    DELETED,
    UNDELIVERABLE,
    ACCEPTED,
    UNKNOWN,
    REJECTED,
    ERROR
  }


  public static class DeliveryState {
    private final State state;
    private final String smppCode;
    private final String smppCodeDescription;

    public DeliveryState(State state, String smppCode, String smppCodeDescription) {
      this.state = state;
      this.smppCode = smppCode;
      this.smppCodeDescription = smppCodeDescription;
    }

    public State getState() {
      return state;
    }

    public String getSmppCode() {
      return smppCode;
    }

    public String getSmppCodeDescription() {
      return smppCodeDescription;
    }

    @Override
    public String toString() {
      final StringBuilder sb = new StringBuilder();
      sb.append("DeliveryState");
      sb.append("{state=").append(state);
      sb.append(", smppCode='").append(smppCode).append('\'');
      sb.append(", smppCodeDescription='").append(smppCodeDescription).append('\'');
      sb.append('}');
      return sb.toString();
    }
  }
}
