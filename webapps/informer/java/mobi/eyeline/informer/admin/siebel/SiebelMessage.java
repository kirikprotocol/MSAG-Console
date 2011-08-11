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

  private String state;
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

  public String getMessageState() {
    return state;
  }

  public void setMessageState(String state) {
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


}
