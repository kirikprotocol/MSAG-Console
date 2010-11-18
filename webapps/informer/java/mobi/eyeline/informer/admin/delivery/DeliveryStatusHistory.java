package mobi.eyeline.informer.admin.delivery;

import java.util.Date;
import java.util.List;

/**
 * История изменения статуса рассылки
 *
 * @author Aleksandr Khalitov
 */
public class DeliveryStatusHistory {

  private final List<Item> items;

  private final int deliveryId;

  DeliveryStatusHistory(int deliveryId, List<Item> items) {
    this.deliveryId = deliveryId;
    this.items = items;
  }

  public int getDeliveryId() {
    return deliveryId;
  }

  public List<Item> getHistoryItems() {
    return items;
  }

  public static class Item {

    private Date date;
    private DeliveryStatus status;

    Item(Date date, DeliveryStatus status) {
      this.date = date;
      this.status = status;
    }

    public Date getDate() {
      return date;
    }

    public DeliveryStatus getStatus() {
      return status;
    }
  }

}
