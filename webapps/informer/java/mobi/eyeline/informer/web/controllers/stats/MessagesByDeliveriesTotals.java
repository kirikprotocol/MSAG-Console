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
  private long deliveredSms;
  private long failedMessages;
  private long failedSms;
  private long expiredMessages;
  private long expiredSms;
  private long newSms;
  private long processSms;
  private long retryMessages;

  public MessagesByDeliveriesTotals() {
    reset();
  }

  public void reset() {
    newMessages=0;
    processMessages=0;
    deliveredMessages=0;
    deliveredSms=0;
    failedMessages=0;
    failedSms=0;
    expiredMessages=0;
    expiredSms=0;
    retryMessages = 0;
    newSms = 0;
    processSms = 0;
  }

  public void add(AggregatedRecord ar) {
    MessagesByDeliveriesRecord r = (MessagesByDeliveriesRecord) ar;
    newMessages+=r.getNewMessages();
    processMessages+=r.getProcMessages();
    deliveredMessages+=r.getDeliveredMessages();
    deliveredSms+=r.getDeliveredSms();
    failedMessages+=r.getFailedMessages();
    failedSms+=r.getFailedSms();
    expiredMessages+=r.getExpiredMessages();
    expiredSms+=r.getExpiredSms();
    processSms += r.getProcessSms();
    newSms += r.getNewSms();
    retryMessages += r.getRetryMessages();
  }

  public long getNewSms() {
    return newSms;
  }

  public long getProcessSms() {
    return processSms;
  }

  public long getRetryMessages() {
    return retryMessages;
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

  public long getDeliveredSms() {
    return deliveredSms;
  }

  public long getFailedSms() {
    return failedSms;
  }

  public long getExpiredSms() {
    return expiredSms;
  }
}
