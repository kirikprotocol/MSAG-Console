package mobi.eyeline.informer.admin.siebel;

import java.util.Date;

/**
 * Рассылка Siebel
 * @author Aleksandr Khalitov
 */
public class SiebelDelivery {

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

  public boolean isSave() {
    return save;
  }

  public void setSave(boolean save) {
    this.save = save;
  }

  public boolean isBeep() {
    return beep;
  }

  public void setBeep(boolean beep) {
    this.beep = beep;
  }

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

  @Override
  public String toString() {
    final StringBuilder sb = new StringBuilder();
    sb.append("SiebelDelivery");
    sb.append("{waveId='").append(waveId).append('\'');
    sb.append(", campaignId='").append(campaignId).append('\'');
    sb.append(", created=").append(created);
    sb.append(", lastUpdate=").append(lastUpdate);
    sb.append(", priority=").append(priority);
    sb.append(", flash=").append(flash);
    sb.append(", save=").append(save);
    sb.append(", beep=").append(beep);
    sb.append(", expPeriod=").append(expPeriod);
    sb.append(", status=").append(status);
    sb.append('}');
    return sb.toString();
  }

  public static enum Status {

    ENQUEUED(true),
    PAUSED(true),
    STOPPED(true),
    IN_PROCESS(false),
    PROCESSED(false);

    private final boolean createdBySiebel;

    private Status(boolean createdBySiebel) {
      this.createdBySiebel = createdBySiebel;
    }

    public boolean isCreatedBySiebel() {
      return createdBySiebel;
    }

  }


}
