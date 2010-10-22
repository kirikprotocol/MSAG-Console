package mobi.eyeline.informer.admin.delivery;

/**
 * Статистика по рассылке
 *
 * @author Aleksandr Khalitov
 */
public class DeliveryStatistics {

  private DeliveryState deliveryState;

  private long newMessages;

  private long processMessage;

  private long deliveredMessages;

  private long failedMessage;

  private long expiredMessages;

  public DeliveryState getDeliveryState() {
    return deliveryState == null ? null : deliveryState.cloneState();
  }

  void setDeliveryState(DeliveryState deliveryState) {
    this.deliveryState = deliveryState;
  }

  public long getNewMessages() {
    return newMessages;
  }

  void setNewMessages(long newMessages) {
    this.newMessages = newMessages;
  }

  public long getProcessMessage() {
    return processMessage;
  }

  void setProcessMessage(long processMessage) {
    this.processMessage = processMessage;
  }

  public long getDeliveredMessages() {
    return deliveredMessages;
  }

  void setDeliveredMessages(long deliveredMessages) {
    this.deliveredMessages = deliveredMessages;
  }

  public long getFailedMessage() {
    return failedMessage;
  }

  void setFailedMessage(long failedMessage) {
    this.failedMessage = failedMessage;
  }

  public long getExpiredMessages() {
    return expiredMessages;
  }

  void setExpiredMessages(long expiredMessages) {
    this.expiredMessages = expiredMessages;
  }
}
