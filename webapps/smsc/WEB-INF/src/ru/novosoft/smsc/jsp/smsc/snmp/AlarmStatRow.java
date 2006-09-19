package ru.novosoft.smsc.jsp.smsc.snmp;

import java.util.Date;

public class AlarmStatRow {
    private Date submit_time;
    private String alarm_id;
    private String alarm_category;
    private int severity;
    private String text;

    public Date getSubmit_time() {
        return submit_time;
    }

    public void setSubmit_time(Date submit_time) {
        this.submit_time = submit_time;
    }

    public String getAlarm_id() {
        return alarm_id;
    }

    public void setAlarm_id(String alarm_id) {
        this.alarm_id = alarm_id;
    }

    public String getAlarm_category() {
        return alarm_category;
    }

    public void setAlarm_category(String alarm_category) {
        this.alarm_category = alarm_category;
    }

    public int getSeverity() {
        return severity;
    }

    public void setSeverity(int severity) {
        this.severity = severity;
    }

    public String getText() {
        return text;
    }

    public void setText(String text) {
        this.text = text;
    }
}
