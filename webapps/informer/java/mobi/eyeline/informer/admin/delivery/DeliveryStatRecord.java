package mobi.eyeline.informer.admin.delivery;

import java.util.Date;

/**
 * Запись в файле со статистикой
 * @author Artem Snopkov
 */
public class DeliveryStatRecord {
  private Date date;

  private int taskId;
  private long newmessages;
  private long processing;
  private long delivered;
  private long failed;
  private long expired;
  private long deliveredSMS;
  private long failedSMS;
  private long expiredSMS;
  private String user;



  public DeliveryStatRecord(String user, Date date, int taskId, int newmessages, int processing, int delivered, int failed, int expired, int deliveredSMS, int failedSMS, int expiredSMS) {
    this.user=user;
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
}
