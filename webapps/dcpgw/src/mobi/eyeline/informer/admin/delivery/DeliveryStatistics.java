package mobi.eyeline.informer.admin.delivery;

/**
 * Статистика по рассылке
 *
 * @author Aleksandr Khalitov
 */
public class DeliveryStatistics {

  private DeliveryState deliveryState;

  private long newMessages;

  private long processMessages;

  private long deliveredMessages;

  private long failedMessages;

  private long expiredMessages;

  private long sentMessages;

  private long retriedMessages;

  public DeliveryState getDeliveryState() {
    return deliveryState == null ? null : deliveryState.cloneState();
  }

  public void setDeliveryState(DeliveryState deliveryState) {
    this.deliveryState = deliveryState;
  }

  public long getNewMessages() {
    return newMessages;
  }

  void setNewMessages(long newMessages) {
    this.newMessages = newMessages;
  }

  public long getProcessMessages() {
    return processMessages;
  }

  void setProcessMessages(long processMessages) {
    this.processMessages = processMessages;
  }

  public long getDeliveredMessages() {
    return deliveredMessages;
  }

  void setDeliveredMessages(long deliveredMessages) {
    this.deliveredMessages = deliveredMessages;
  }

  public long getFailedMessages() {
    return failedMessages;
  }

  void setFailedMessages(long failedMessages) {
    this.failedMessages = failedMessages;
  }

  public long getExpiredMessages() {
    return expiredMessages;
  }

  void setExpiredMessages(long expiredMessages) {
    this.expiredMessages = expiredMessages;
  }

  public long getSentMessages() {
    return sentMessages;
  }

  void setSentMessages(long sentMessages) {
    this.sentMessages = sentMessages;
  }

  public long getRetriedMessages() {
    return retriedMessages;
  }

  void setRetriedMessages(long retriedMessages) {
    this.retriedMessages = retriedMessages;
  }
}
