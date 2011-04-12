package mobi.eyeline.informer.web.controllers.stats;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 19.11.2010
 * Time: 16:52:18
 */
public class MessagesByPeriodTotals implements DeliveryStatTotals {

  private long newMessages;
  private long processMessages;
  private long deliveredMessages;
  private long failedMessages;
  private long expiredMessages;
  private long deliveredMessagesSMS;
  private long failedMessagesSMS;
  private long expiredMessagesSMS;

  public MessagesByPeriodTotals() {
    reset();
  }

  public void reset() {
    this.newMessages = 0;
    this.processMessages = 0;
    this.deliveredMessages = 0;
    this.failedMessages = 0;
    this.expiredMessages = 0;
    this.deliveredMessagesSMS = 0;
    this.failedMessagesSMS = 0;
    this.expiredMessagesSMS = 0;
  }

  public void add(AggregatedRecord r) {
    if(r instanceof MessagesByPeriodRecord){
      MessagesByPeriodRecord mpr = (MessagesByPeriodRecord) r;
      this.newMessages+=mpr.getNewMessages();
      this.processMessages += mpr.getProcessMessages();
      this.deliveredMessages += mpr.getDeliveredMessages();
      this.failedMessages += mpr.getFailedMessages();
      this.expiredMessages += mpr.getExpiredMessages();
      this.deliveredMessagesSMS += mpr.getDeliveredMessagesSMS();
      this.failedMessagesSMS += mpr.getFailedMessagesSMS();
      this.expiredMessagesSMS += mpr.getExpiredMessagesSMS();
    }else if(r instanceof MessagesByRegionRecord) {
      MessagesByRegionRecord mpr = (MessagesByRegionRecord) r;
      this.newMessages+=mpr.getNewMessages();
      this.processMessages += mpr.getProcessMessages();
      this.deliveredMessages += mpr.getDeliveredMessages();
      this.failedMessages += mpr.getFailedMessages();
      this.expiredMessages += mpr.getExpiredMessages();
      this.deliveredMessagesSMS += mpr.getDeliveredMessagesSMS();
      this.failedMessagesSMS += mpr.getFailedMessagesSMS();
      this.expiredMessagesSMS += mpr.getExpiredMessagesSMS();
    }
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

  public long getDeliveredMessagesSMS() {
    return deliveredMessagesSMS;
  }

  public long getFailedMessagesSMS() {
    return failedMessagesSMS;
  }

  public long getExpiredMessagesSMS() {
    return expiredMessagesSMS;
  }

}
