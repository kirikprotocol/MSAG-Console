package mobi.eyeline.informer.admin.delivery;

import java.util.Date;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 18:29:04
 */
public class DeliveryNotification {
  private final Date eventDate;
  private final int deliveryId;
  private final String userId;
  private final DeliveryNotificationType type;


  public DeliveryNotification(DeliveryNotificationType type, Date eventDate, int deliveryId, String userId) {
    this.type = type;
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

  public DeliveryNotificationType getType() {
    return type;
  }
}
