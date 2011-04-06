package mobi.eyeline.informer.admin.delivery.stat;

import java.util.Date;

/**
 * Запись в файле со статистикой
 *
 * @author Artem Snopkov
 */
public class DeliveryStatRecord {
  private final Date date;

  private final int taskId;
  private final long newmessages;
  private final long processing;
  private final long delivered;
  private final long failed;
  private final long expired;
  private final long deliveredSMS;
  private final long failedSMS;
  private final long expiredSMS;
  private final String user;
  private final Integer regionId;


  public DeliveryStatRecord(String user, Date date, int taskId, int newmessages, int processing, int delivered, int failed, int expired, int deliveredSMS, int failedSMS, int expiredSMS, Integer regionId) {
    this.user = user;
    this.date = date;
    this.taskId = taskId;
    this.newmessages = newmessages;
    this.processing = processing;
    this.delivered = delivered;
    this.failed = failed;
    this.expired = expired;
    this.deliveredSMS = deliveredSMS;
    this.failedSMS = failedSMS;
    this.expiredSMS = expiredSMS;
    this.regionId = regionId;
  }

  public Date getDate() {
    return date;
  }

  public int getTaskId() {
    return taskId;
  }

  public long getNewmessages() {
    return newmessages;
  }

  public long getProcessing() {
    return processing;
  }

  public long getDelivered() {
    return delivered;
  }

  public long getFailed() {
    return failed;
  }

  public long getExpired() {
    return expired;
  }

  public long getDeliveredSMS() {
    return deliveredSMS;
  }

  public long getFailedSMS() {
    return failedSMS;
  }

  public long getExpiredSMS() {
    return expiredSMS;
  }

  public String getUser() {
    return user;
  }

  public Integer getRegionId() {
    return regionId;
  }
}
