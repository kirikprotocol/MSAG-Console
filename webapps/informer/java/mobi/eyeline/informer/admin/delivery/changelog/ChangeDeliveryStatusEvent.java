package mobi.eyeline.informer.admin.delivery.changelog;

import mobi.eyeline.informer.admin.delivery.DeliveryStatus;

import java.util.Date;

/**
 * Нотификация об измеении статуса рассылки
 *
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 18:29:04
 */
public class ChangeDeliveryStatusEvent {
  private final Date eventDate;
  private final int deliveryId;
  private final String userId;
  private final DeliveryStatus status;


  public ChangeDeliveryStatusEvent(DeliveryStatus status, Date eventDate, int deliveryId, String userId) {
    this.status = status;
    this.eventDate = eventDate;
    this.deliveryId = deliveryId;
    this.userId = userId;
  }

  public Date getEventDate() {
    return eventDate;
  }

  public int getDeliveryId() {
    return deliveryId;
  }

  public String getUserId() {
    return userId;
  }

  public DeliveryStatus getStatus() {
    return status;
  }
}
