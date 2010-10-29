package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.DeliveryStatistics;
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
public class MessagesByUserStatRecord extends AggregatedRecord {
  String userId;
  private long newMessages;
  private long processMessages;
  private long deliveredMessages;
  private long failedMessages;
  private long expiredMessages;

  public MessagesByUserStatRecord(String userId, DeliveryStatistics stat) {
    this.userId = userId;
    this.newMessages = stat.getNewMessages();
    this.processMessages = stat.getProcessMessages();
    this.deliveredMessages= stat.getDeliveredMessages();
    this.failedMessages= stat.getFailedMessages();
    this.expiredMessages= stat.getExpiredMessages();
  }


  @Override
  Object getAggregationKey() {
    return userId;
  }

  @Override
  void add(AggregatedRecord other) {
     MessagesByUserStatRecord r = (MessagesByUserStatRecord) other;
     this.newMessages+=r.newMessages;
     this.processMessages+=r.processMessages;
     this.deliveredMessages+=r.deliveredMessages;
     this.failedMessages+=r.failedMessages;
     this.expiredMessages+=r.expiredMessages;
  }

  @Override
  Comparator getRecordsComparator(final DataTableSortOrder sortOrder) {
    return new Comparator<MessagesByUserStatRecord>() {
         public int compare(MessagesByUserStatRecord o1, MessagesByUserStatRecord o2) {

              final int mul = sortOrder.isAsc() ? 1 : -1;
              if (sortOrder.getColumnId().equals("userId")) {
                return mul*o1.getUserId().compareTo(o2.getUserId());
              }
              else if (sortOrder.getColumnId().equals("new")) {
                return o1.getNewMessages() >= o2.getNewMessages() ? mul : -mul;
              }
              else if (sortOrder.getColumnId().equals("process")) {
                return o1.getProcessMessages() >= o2.getProcessMessages() ? mul : -mul;
              }
              else if (sortOrder.getColumnId().equals("delivered")) {
                return o1.getDeliveredMessages() >= o2.getDeliveredMessages() ? mul : -mul;
              }
              else if (sortOrder.getColumnId().equals("falied")) {
                return o1.getFailedMessages() >= o2.getFailedMessages() ? mul : -mul;
              }
              else if (sortOrder.getColumnId().equals("expired")) {
                return o1.getExpiredMessages() >= o2.getExpiredMessages() ? mul : -mul;
              }
              return 0;
            }
    };
  }

  @Override
  void printWithChildrenToCSV(PrintWriter writer) {
     writer.println(StringEncoderDecoder.toCSVString(
         new Object[]{userId,newMessages,processMessages,deliveredMessages,failedMessages,expiredMessages}
     ));
  }

  public String getUserId() {
    return userId;
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
