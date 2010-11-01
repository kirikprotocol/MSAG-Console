package mobi.eyeline.informer.admin.delivery;

import java.util.Date;

/**
 * Запись в файле со статистикой
 * @author Artem Snopkov
 */
public class DeliveryStatRecord {
  private Date date;

  private int taskId;
  private int newmessages;
  private int processing;
  private int delivered;
  private int failed;
  private int expired;
  private int deliveredSMS;
  private int failedSMS;
  private int expiredSMS;
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

  public int getNewmessages() {
    return newmessages;
  }

  public int getProcessing() {
    return processing;
  }

  public int getDelivered() {
    return delivered;
  }

  public int getFailed() {
    return failed;
  }

  public int getExpired() {
    return expired;
  }

  public int getDeliveredSMS() {
    return deliveredSMS;
  }

  public int getFailedSMS() {
    return failedSMS;
  }

  public int getExpiredSMS() {
    return expiredSMS;
  }

  public String getUser() {
    return user;
  }
}
