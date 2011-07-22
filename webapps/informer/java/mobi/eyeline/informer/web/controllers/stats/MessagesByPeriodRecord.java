package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatRecord;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.PrintWriter;
import java.util.Comparator;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 16:22:41
 */
public class MessagesByPeriodRecord extends TimeAggregatedStatRecord implements MessagesRecord{

  private long newMessages;
  private long processMessages;
  private long deliveredMessages;
  private long failedMessages;
  private long expiredMessages;
  private long deliveredMessagesSMS;
  private long failedMessagesSMS;
  private long expiredMessagesSMS;
  private long newSms;
  private long processSms;
  private long retryMessages;

  public MessagesByPeriodRecord(DeliveryStatRecord dsr, AggregationType type, boolean isParent) {
    super(dsr.getDate(), type, isParent);
    this.newMessages = dsr.getNewmessages();
    this.processMessages = dsr.getProcessing();
    this.deliveredMessages = dsr.getDelivered();
    this.failedMessages = dsr.getFailed();
    this.expiredMessages = dsr.getExpired();
    this.deliveredMessagesSMS = dsr.getDeliveredSMS();
    this.failedMessagesSMS = dsr.getFailedSMS();
    this.expiredMessagesSMS = dsr.getExpiredSMS();
    this.processSms = dsr.getProcessingSms();
    this.newSms = dsr.getNewSms();
    this.retryMessages = dsr.getRetry();

    if (getChildAggregationType() != null) {
      MessagesByPeriodRecord child = new MessagesByPeriodRecord(dsr, getChildAggregationType(), false);
      addChild(child);
    }
  }


  public void add(AggregatedRecord r) {
    MessagesByPeriodRecord other = (MessagesByPeriodRecord) r;
    this.newMessages += other.getNewMessages();
    this.processMessages += other.getProcessMessages();
    this.deliveredMessages += other.getDeliveredMessages();
    this.failedMessages += other.getFailedMessages();
    this.expiredMessages += other.getExpiredMessages();
    this.deliveredMessagesSMS += other.getDeliveredMessagesSMS();
    this.failedMessagesSMS += other.getFailedMessagesSMS();
    this.expiredMessagesSMS += other.getExpiredMessagesSMS();
    this.processSms += other.getProcessSms();
    this.newSms += other.getNewSms();
    this.retryMessages += other.getRetryMessages();
    addChildren(r);
  }


  void printCSV(PrintWriter writer) {

      if (isParent()) {
        writer.println(StringEncoderDecoder.toCSVString(';',
            getPeriodString(), "",
            getNewMessages(), getNewSms(),
            getRetryMessages(),
            getProcessMessages(), getProcessSms(),
            getDeliveredMessages(), getDeliveredMessagesSMS(),
            getFailedMessages(), getFailedMessagesSMS(),
            getExpiredMessages(), getExpiredMessagesSMS()));
      } else {
        writer.println(StringEncoderDecoder.toCSVString(';',
            "", getPeriodString(),
            getNewMessages(), getNewSms(),
            getRetryMessages(),
            getProcessMessages(), getProcessSms(),
            getDeliveredMessages(), getDeliveredMessagesSMS(),
            getFailedMessages(), getFailedMessagesSMS(),
            getExpiredMessages(), getExpiredMessagesSMS()));
      }

  }

  @Override
  void printCSVheader(PrintWriter writer) {
    writer.println(StringEncoderDecoder.toCSVString(';',
        "PERIOD", "",
        "NEW", "NEW SMS",
        "RETRY",
        "PROCESS", "PROCESS SMS",
        "DELIVERED", "DELIVERED SMS",
        "FAILED", "FAILED SMS",
        "EXPIRED", "EXPIRED SMS"));
  }


  public Comparator getRecordsComparator(final DataTableSortOrder sortOrder) {

    return new Comparator<MessagesByPeriodRecord>() {
      public int compare(MessagesByPeriodRecord o1, MessagesByPeriodRecord o2) {
        final int mul = sortOrder.isAsc() ? 1 : -1;
        if (sortOrder.getColumnId().equals("period")) {
          return mul * o1.getStartCalendar().compareTo(o2.getStartCalendar());
        } else if (sortOrder.getColumnId().equals("new")) {
          return o1.getNewMessages() >= o2.getNewMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("retry")) {
          return o1.getRetryMessages() >= o2.getRetryMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("process")) {
          return o1.getProcessMessages() >= o2.getProcessMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("delivered")) {
          return o1.getDeliveredMessages() >= o2.getDeliveredMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("failed")) {
          return o1.getFailedMessages() >= o2.getFailedMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("expired")) {
          return o1.getExpiredMessages() >= o2.getExpiredMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("wait")) {
          return o1.getNewMessages() + o1.getProcessMessages() >= o2.getNewMessages() + o2.getProcessMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("notDelivered")) {
          return o1.getFailedMessages() + o1.getExpiredMessages() >= o2.getFailedMessages() + o2.getExpiredMessages() ? mul : -mul;
        }
        return 0;
      }
    };
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
