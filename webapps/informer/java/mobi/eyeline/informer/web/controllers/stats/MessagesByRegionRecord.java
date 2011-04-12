package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatRecord;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.PrintWriter;
import java.util.Comparator;

/**
 * @author Aleksandr Khalitov
 */
public class MessagesByRegionRecord extends AggregatedRecord{

  private long newMessages;
  private long processMessages;
  private long deliveredMessages;
  private long failedMessages;
  private long expiredMessages;
  private long deliveredMessagesSMS;
  private long failedMessagesSMS;
  private long expiredMessagesSMS;

  private String region;
  private Integer regionId;

  private boolean deleted;

  public MessagesByRegionRecord(DeliveryStatRecord dsr, String region, boolean deleted) {
    this.newMessages = dsr.getNewmessages();
    this.processMessages = dsr.getProcessing();
    this.deliveredMessages = dsr.getDelivered();
    this.failedMessages = dsr.getFailed();
    this.expiredMessages = dsr.getExpired();
    this.deliveredMessagesSMS = dsr.getDeliveredSMS();
    this.failedMessagesSMS = dsr.getFailedSMS();
    this.expiredMessagesSMS = dsr.getExpiredSMS();
    this.region = region;
    this.regionId = dsr.getRegionId();
    this.deleted = deleted;
  }

  public boolean isDeleted() {
    return deleted;
  }

  @Override
  Object getAggregationKey() {
    return region;
  }

  @Override
  void add(AggregatedRecord r) {
    MessagesByRegionRecord other = (MessagesByRegionRecord) r;
    this.newMessages += other.getNewMessages();
    this.processMessages += other.getProcessMessages();
    this.deliveredMessages += other.getDeliveredMessages();
    this.failedMessages += other.getFailedMessages();
    this.expiredMessages += other.getExpiredMessages();
    this.deliveredMessagesSMS += other.getDeliveredMessagesSMS();
    this.failedMessagesSMS += other.getFailedMessagesSMS();
    this.expiredMessagesSMS += other.getExpiredMessagesSMS();
  }


  @Override
  Comparator getRecordsComparator(final DataTableSortOrder sortOrder) {

    return new Comparator<MessagesByRegionRecord>() {
      public int compare(MessagesByRegionRecord o1, MessagesByRegionRecord o2) {
        final int mul = sortOrder.isAsc() ? 1 : -1;
        if (sortOrder.getColumnId().equals("region")) {
          return mul * o1.getRegion().compareTo(o2.getRegion());
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
          "REGION",
          "NEW",
          "PROCESS",
          "DELIVERED", "DELIVERED SMS",
          "FAILED", "FAILED SMS",
          "EXPIRED", "EXPIRED SMS"));
    } else {
      writer.println(StringEncoderDecoder.toCSVString(';',
          "REGION",
          "WAIT",
          "DELIVERED", "DELIVERED SMS",
          "NOT DELIVERED", "NOT DELIVERED SMS"));
    }

  }

  @Override
  void printWithChildrenToCSV(PrintWriter writer, boolean detalised) {
    if (detalised) {
      writer.println(StringEncoderDecoder.toCSVString(';',
          getRegion(),
          getNewMessages(),
          getProcessMessages(),
          getDeliveredMessages(), getDeliveredMessagesSMS(),
          getFailedMessages(), getFailedMessagesSMS(),
          getExpiredMessages(), getExpiredMessagesSMS()));
    }
  }

  public Integer getRegionId() {
    return regionId;
  }

  public void setRegionId(Integer regionId) {
    this.regionId = regionId;
  }

  public long getNewMessages() {
    return newMessages;
  }

  public void setNewMessages(long newMessages) {
    this.newMessages = newMessages;
  }

  public long getProcessMessages() {
    return processMessages;
  }

  public void setProcessMessages(long processMessages) {
    this.processMessages = processMessages;
  }

  public long getDeliveredMessages() {
    return deliveredMessages;
  }

  public void setDeliveredMessages(long deliveredMessages) {
    this.deliveredMessages = deliveredMessages;
  }

  public long getFailedMessages() {
    return failedMessages;
  }

  public void setFailedMessages(long failedMessages) {
    this.failedMessages = failedMessages;
  }

  public long getExpiredMessages() {
    return expiredMessages;
  }

  public void setExpiredMessages(long expiredMessages) {
    this.expiredMessages = expiredMessages;
  }

  public long getDeliveredMessagesSMS() {
    return deliveredMessagesSMS;
  }

  public void setDeliveredMessagesSMS(long deliveredMessagesSMS) {
    this.deliveredMessagesSMS = deliveredMessagesSMS;
  }

  public long getFailedMessagesSMS() {
    return failedMessagesSMS;
  }

  public void setFailedMessagesSMS(long failedMessagesSMS) {
    this.failedMessagesSMS = failedMessagesSMS;
  }

  public long getExpiredMessagesSMS() {
    return expiredMessagesSMS;
  }

  public void setExpiredMessagesSMS(long expiredMessagesSMS) {
    this.expiredMessagesSMS = expiredMessagesSMS;
  }

  public String getRegion() {
    return region;
  }
}
