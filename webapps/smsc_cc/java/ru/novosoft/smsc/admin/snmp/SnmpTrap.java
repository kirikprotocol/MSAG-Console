package ru.novosoft.smsc.admin.snmp;

import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class SnmpTrap {

  private Date submitDate;

  private String alarmId;

  private String alarmCategory;

  private Integer severity;

  private String text;

  public SnmpTrap() {
  }

  public SnmpTrap(SnmpTrap t) {
    this.submitDate = t.submitDate == null ? null : new Date(t.submitDate.getTime());
    this.alarmId = t.alarmId;
    this.alarmCategory = t.alarmCategory;
    this.severity = t.severity;
    this.text = t.text;
  }

  public Date getSubmitDate() {
    return submitDate;
  }

  public void setSubmitDate(Date submitDate) {
    this.submitDate = submitDate;
  }

  public String getAlarmId() {
    return alarmId;
  }

  public void setAlarmId(String alarmId) {
    this.alarmId = alarmId;
  }

  public String getAlarmCategory() {
    return alarmCategory;
  }

  public void setAlarmCategory(String alarmCategory) {
    this.alarmCategory = alarmCategory;
  }

  public Integer getSeverity() {
    return severity;
  }

  public void setSeverity(Integer severity) {
    this.severity = severity;
  }

  public String getText() {
    return text;
  }

  public void setText(String text) {
    this.text = text;
  }

  @Override
  public String toString() {
    final StringBuilder sb = new StringBuilder();
    sb.append("SnmpRecord");
    sb.append("{submitDate=").append(submitDate);
    sb.append(", alarmId='").append(alarmId).append('\'');
    sb.append(", alarmCategory='").append(alarmCategory).append('\'');
    sb.append(", severity=").append(severity);
    sb.append(", text='").append(text).append('\'');
    sb.append('}');
    return sb.toString();
  }
}
