package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.DeliveryStatRecord;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.PrintWriter;
import java.util.Comparator;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 29.10.2010
 * Time: 13:28:45
 */
public class MessagesByUserStatTotals implements DeliveryStatTotals{
  private long newMessages;
  private long processMessages;
  private long deliveredMessages;
  private long failedMessages;
  private long expiredMessages;
  private long deliveredMessagesSMS;
  private long failedMessagesSMS;
  private long expiredMessagesSMS;

  public MessagesByUserStatTotals() {
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
    MessagesByUserStatRecord mur = (MessagesByUserStatRecord) r;
    this.newMessages+=mur.getNewMessages();
    this.processMessages += mur.getProcessMessages();
    this.deliveredMessages += mur.getDeliveredMessages();
    this.failedMessages += mur.getFailedMessages();
    this.expiredMessages += mur.getExpiredMessages();
    this.deliveredMessagesSMS += mur.getDeliveredMessagesSMS();
    this.failedMessagesSMS += mur.getFailedMessagesSMS();
    this.expiredMessagesSMS += mur.getExpiredMessagesSMS();
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