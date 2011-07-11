package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatRecord;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.PrintWriter;
import java.util.Comparator;

/**
 * @author Aleksandr Khalitov
 */
public class MessagesBySmscRecord extends AggregatedRecord implements MessagesRecord{

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

  private String smsc;

  private boolean deleted;

  public MessagesBySmscRecord(DeliveryStatRecord dsr, String smsc, boolean deleted) {
    this.newMessages = dsr.getNewmessages();
    this.processMessages = dsr.getProcessing();
    this.deliveredMessages = dsr.getDelivered();
    this.failedMessages = dsr.getFailed();
    this.expiredMessages = dsr.getExpired();
    this.deliveredMessagesSMS = dsr.getDeliveredSMS();
    this.failedMessagesSMS = dsr.getFailedSMS();
    this.expiredMessagesSMS = dsr.getExpiredSMS();
    this.smsc = smsc;
    this.deleted = deleted;
    this.processSms = dsr.getProcessingSms();
    this.newSms = dsr.getNewSms();
    this.retryMessages = dsr.getRetry();
  }

  public boolean isDeleted() {
    return deleted;
  }

  @Override
  Object getAggregationKey() {
    return smsc;
  }

  @Override
  void add(AggregatedRecord r) {
    MessagesBySmscRecord other = (MessagesBySmscRecord) r;
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
  }


  @Override
  Comparator getRecordsComparator(final DataTableSortOrder sortOrder) {
    return new Comparator<MessagesBySmscRecord>() {
      public int compare(MessagesBySmscRecord o1, MessagesBySmscRecord o2) {
        final int mul = sortOrder.isAsc() ? 1 : -1;
        if (sortOrder.getColumnId().equals("smsc")) {
          return mul * o1.getSmsc().compareTo(o2.getSmsc());
        } else if (sortOrder.getColumnId().equals("new")) {
          return o1.getNewMessages() >= o2.getNewMessages() ? mul : -mul;
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

  @Override
  void printCSVheader(PrintWriter writer, boolean detalized) {
    if (detalized) {
      writer.println(StringEncoderDecoder.toCSVString(';',
          "SMSC",
          "NEW", "NEW SMS",
          "PROCESS", "PROCESS SMS",
          "DELIVERED", "DELIVERED SMS",
          "FAILED", "FAILED SMS",
          "EXPIRED", "EXPIRED SMS"));
    } else {
      writer.println(StringEncoderDecoder.toCSVString(';',
          "SMSC",
          "WAIT",
          "DELIVERED", "DELIVERED SMS",
          "NOT DELIVERED", "NOT DELIVERED SMS"));
    }

  }

  @Override
  void printWithChildrenToCSV(PrintWriter writer, boolean detalised) {
    if (detalised) {
      writer.println(StringEncoderDecoder.toCSVString(';',
          getSmsc(),
          getNewMessages(), getNewSms(),
          getProcessMessages(), getProcessSms(),
          getDeliveredMessages(), getDeliveredMessagesSMS(),
          getFailedMessages(), getFailedMessagesSMS(),
          getExpiredMessages(), getExpiredMessagesSMS()));
    }else {
      writer.println(StringEncoderDecoder.toCSVString(';',
          getSmsc(),
          getNewMessages() + getProcessMessages(),
          getDeliveredMessages(), getDeliveredMessagesSMS(),
          getFailedMessages() + getExpiredMessages(), getFailedMessagesSMS() + getExpiredMessagesSMS()));
    }
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

  public String getSmsc() {
    return smsc;
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
