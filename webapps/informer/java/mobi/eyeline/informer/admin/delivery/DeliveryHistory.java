package mobi.eyeline.informer.admin.delivery;

import java.util.Date;
import java.util.List;

/**
 * История изменения статуса рассылки
 * @author Aleksandr Khalitov
 */
public class DeliveryHistory {

  private final List<HistoryItem> items;

  private final int deliveryId;

  DeliveryHistory(int deliveryId, List<HistoryItem> items) {
    this.deliveryId = deliveryId;
    this.items = items;
  }

  public int getDeliveryId() {
    return deliveryId;
  }

  public List<HistoryItem> getHistoryItems() {
    return items;
  }

  public static class HistoryItem {

    private Date date;
    private DeliveryStatus status;

    HistoryItem(Date date, DeliveryStatus status) {
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
