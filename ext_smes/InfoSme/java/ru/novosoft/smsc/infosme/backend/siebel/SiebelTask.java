package ru.novosoft.smsc.infosme.backend.siebel;

import java.util.Date;

/**
 * author: alkhal
 */

public class SiebelTask {

  private String waveId;   //unique
  private String campaignId;
  private Date created;
  private Date lastUpdate;
  private int priority;
  private boolean flash;
  private boolean save;
  private boolean beep;
  private Integer expPeriod;
  private Status status;

  public String getWaveId() {
    return waveId;
  }

  public void setWaveId(String waveId) {
    this.waveId = waveId;
  }

  public String getCampaignId() {
    return campaignId;
  }

  public void setCampaignId(String campaignId) {
    this.campaignId = campaignId;
  }

  public Date getCreated() {
    return created;
  }

  public void setCreated(Date created) {
    this.created = created;
  }

  public Date getLastUpdate() {
    return lastUpdate;
  }

  public void setLastUpdate(Date lastUpdate) {
    this.lastUpdate = lastUpdate;
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) {
    this.priority = priority;
  }

  public boolean isFlash() {
    return flash;
  }

  public void setFlash(boolean flash) {
    this.flash = flash;
  }

//  public void setFlash(String flash) {
//    this.flash = flash.equalsIgnoreCase("Y");
//  }

  public boolean isSave() {
    return save;
  }

  public void setSave(boolean save) {
    this.save = save;
  }

//  public void setSave(String save) {
//    this.save = save.equalsIgnoreCase("Y");
//  }

  public boolean isBeep() {
    return beep;
  }

  public void setBeep(boolean beep) {
    this.beep = beep;
  }

//  public void setBeep(String beep) {
//    this.beep = beep.equalsIgnoreCase("Y");
//  }

  public Integer getExpPeriod() {
    return expPeriod;
  }

  public void setExpPeriod(Integer expPeriod) {
    this.expPeriod = expPeriod;
  }


  public Status getStatus() {
    return status;
  }

  public void setStatus(Status status) {
    this.status = status;
  }

  public static class Status {

    public final static Status ENQUEUED = new Status("ENQUEUED", true);
    public final static Status PAUSED = new Status("PAUSED", true);
    public final static Status STOPPED = new Status("STOPPED", true);
    public final static Status IN_PROCESS = new Status("IN PROCESS", false);
    public final static Status PROCESSED = new Status("PROCESSED", false);

    private final String value;
    private final boolean createdBySiebel;

    private Status(String value, boolean createdBySiebel) {
      this.value = value;
      this.createdBySiebel = createdBySiebel;
    }

    public String toString() {
      return value;
    }

    public boolean isCreatedBySiebel() {
      return createdBySiebel;
    }

    public static Status valueOf(String st) {
      if (ENQUEUED.value.equals(st)) {
        return ENQUEUED;
      } else if (PAUSED.value.equals(st)) {
        return PAUSED;
      } else if (STOPPED.value.equals(st)) {
        return STOPPED;
      } else if (IN_PROCESS.value.equals(st)) {
        return IN_PROCESS;
      } else if (PROCESSED.value.equals(st)) {
        return PROCESSED;
      } else {
        throw new IllegalArgumentException("Unknown state: " + st);
      }
    }

    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;

      Status that = (Status) o;

      return !(value != null ? !value.equals(that.value) : that.value != null);

    }

  }

  public String toString() {
    return "SiebelTask{" +
        "waveId='" + waveId + '\'' +
        ", campaignId='" + campaignId + '\'' +
        ", created=" + created +
        ", lastUpdate=" + lastUpdate +
        ", priority=" + priority +
        ", flash=" + flash +
        ", save=" + save +
        ", beep=" + beep +
        ", expPeriod=" + expPeriod +
        ", status=" + status +
        '}';
  }
}
