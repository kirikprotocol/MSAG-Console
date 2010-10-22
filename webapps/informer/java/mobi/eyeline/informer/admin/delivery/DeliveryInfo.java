package mobi.eyeline.informer.admin.delivery;

import java.util.Date;

/**
 * Краткая информация о рассылке
 *
 * @author Aleksandr Khalitov
 */
public class DeliveryInfo {

  private int deliveryId;
  private String userId;
  private String name;
  private DeliveryStatus status;
  private Date startDate;
  private Date endDate;
  private Date activityPeriodStart;
  private Date activityPeriodEnd;

  public int getDeliveryId() {
    return deliveryId;
  }

  void setDeliveryId(int deliveryId) {
    this.deliveryId = deliveryId;
  }

  public String getUserId() {
    return userId;
  }

  void setUserId(String userId) {
    this.userId = userId;
  }

  public String getName() {
    return name;
  }

  void setName(String name) {
    this.name = name;
  }

  public DeliveryStatus getStatus() {
    return status;
  }

  void setStatus(DeliveryStatus status) {
    this.status = status;
  }

  public Date getStartDate() {
    return startDate;
  }

  void setStartDate(Date startDate) {
    this.startDate = startDate;
  }

  public Date getEndDate() {
    return endDate;
  }

  void setEndDate(Date endDate) {
    this.endDate = endDate;
  }

  public Date getActivityPeriodStart() {
    return activityPeriodStart;
  }

  void setActivityPeriodStart(Date activityPeriodStart) {
    this.activityPeriodStart = activityPeriodStart;
  }

  public Date getActivityPeriodEnd() {
    return activityPeriodEnd;
  }

  void setActivityPeriodEnd(Date activityPeriodEnd) {
    this.activityPeriodEnd = activityPeriodEnd;
  }
}
