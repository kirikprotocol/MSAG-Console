package mobi.eyeline.informer.admin.delivery;

/**
 * Статистика по рассылке
 * @author Aleksandr Khalitov
 */
public class DeliveryStatistics {

  private DeliveryState deliveryState;

  private int newMessages;

  private int processMessage;

  private int deliveredMessages;

  private int failedMessage;

  private int expiredMessages;

  public DeliveryState getDeliveryState() {
    return deliveryState == null ? null : deliveryState.cloneState();
  }

  void setDeliveryState(DeliveryState deliveryState) {
    this.deliveryState = deliveryState;
  }

  public int getNewMessages() {
    return newMessages;
  }

  void setNewMessages(int newMessages) {
    this.newMessages = newMessages;
  }

  public int getProcessMessage() {
    return processMessage;
  }

  void setProcessMessage(int processMessage) {
    this.processMessage = processMessage;
  }

  public int getDeliveredMessages() {
    return deliveredMessages;
  }

  void setDeliveredMessages(int deliveredMessages) {
    this.deliveredMessages = deliveredMessages;
  }

  public int getFailedMessage() {
    return failedMessage;
  }

  void setFailedMessage(int failedMessage) {
    this.failedMessage = failedMessage;
  }

  public int getExpiredMessages() {
    return expiredMessages;
  }

  void setExpiredMessages(int expiredMessages) {
    this.expiredMessages = expiredMessages;
  }
}
