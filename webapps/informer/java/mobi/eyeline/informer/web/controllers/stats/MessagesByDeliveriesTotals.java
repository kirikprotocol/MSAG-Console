package mobi.eyeline.informer.web.controllers.stats;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 19.11.2010
 * Time: 16:24:10
 */
public class MessagesByDeliveriesTotals implements DeliveryStatTotals{

  private long newMessages;
  private long processMessages;
  private long deliveredMessages;
  private long failedMessages;
  private long expiredMessages;

  public MessagesByDeliveriesTotals() {
    reset();
  }

  public void reset() {
    newMessages=0;
    processMessages=0;
    deliveredMessages=0;
    failedMessages=0;
    expiredMessages=0;
  }

  public void add(AggregatedRecord ar) {
    MessagesByDeliveriesRecord r = (MessagesByDeliveriesRecord) ar;
    newMessages+=r.getNewMessages();
    processMessages+=r.getProcMessages();
    deliveredMessages+=r.getDeliveredMessages();
    failedMessages+=r.getFailedMessages();
    expiredMessages+=r.getExpiredMessages();
  }

  public long getNewMessages() {
    return newMessages;
  }

  public long getProcessMessages() {
    return processMessages;
  }

  public long getDeliveredMessages() {
    return deliveredMessages;
  }

  public long getFailedMessages() {
    return failedMessages;
  }

  public long getExpiredMessages() {
    return expiredMessages;
  }
}
