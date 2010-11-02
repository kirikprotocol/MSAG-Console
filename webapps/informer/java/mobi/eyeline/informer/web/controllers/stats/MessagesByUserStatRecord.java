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
public class MessagesByUserStatRecord extends AggregatedRecord {
  String userId;
  private User user;
  private long newMessages;
  private long processMessages;
  private long deliveredMessages;
  private long failedMessages;
  private long expiredMessages;
  private long deliveredMessagesSMS;
  private long failedMessagesSMS;
  private long expiredMessagesSMS;

  public MessagesByUserStatRecord(String userId, User user, DeliveryStatRecord rec) {
    this.userId = userId;
    this.user=user;
    this.newMessages = rec.getNewmessages();
    this.processMessages = rec.getProcessing();
    this.deliveredMessages= rec.getDelivered();
    this.failedMessages= rec.getFailed();
    this.expiredMessages= rec.getExpired();

    this.deliveredMessagesSMS = rec.getDeliveredSMS();
    this.failedMessagesSMS    = rec.getFailedSMS();
    this.expiredMessagesSMS   = rec.getExpiredSMS();
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
     this.deliveredMessagesSMS+=r.deliveredMessagesSMS;
     this.failedMessages+=r.failedMessages;
     this.failedMessagesSMS+=r.failedMessagesSMS;
     this.expiredMessages+=r.expiredMessages;
     this.expiredMessagesSMS+=r.expiredMessagesSMS;
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
              else if (sortOrder.getColumnId().equals("failed")) {
                return o1.getFailedMessages() >= o2.getFailedMessages() ? mul : -mul;
              }
              else if (sortOrder.getColumnId().equals("expired")) {
                return o1.getExpiredMessages() >= o2.getExpiredMessages() ? mul : -mul;
              }
              else if (sortOrder.getColumnId().equals("wait")) {
                return o1.getNewMessages()+o1.getProcessMessages() >= o2.getNewMessages()+o2.getProcessMessages() ? mul : -mul;
              }
              else if (sortOrder.getColumnId().equals("notDelivered")) {
                return o1.getFailedMessages()+o1.getExpiredMessages() >= o2.getFailedMessages()+o2.getExpiredMessages() ? mul : -mul;
              }
              return 0;
            }
    };
  }

  @Override
  void printCSVheader(PrintWriter writer,boolean detalized) {
    if(detalized) {
      writer.println(StringEncoderDecoder.toCSVString(
         new Object[]{"USER","NEW","PROCESS","DELIVERED","DELIVERED_SMS","FAILED","FAILED_SMS","EXPIRED","EXPIRED_SMS"}
      ));
    }
    else {
      writer.println(StringEncoderDecoder.toCSVString(
      new Object[]{"USER","WAIT","DELIVERED","DELIVERED_SMS","NOTDELIVERED","NOTDELIVERED_SMS"}
      ));
    }
  }


  @Override
  void printWithChildrenToCSV(PrintWriter writer, boolean detalized) {
    if(detalized) {
      writer.println(StringEncoderDecoder.toCSVString(
         new Object[]{userId,newMessages,processMessages,deliveredMessages,deliveredMessagesSMS,failedMessages,failedMessagesSMS,expiredMessages,expiredMessagesSMS}
      ));
    }
    else {
      writer.println(StringEncoderDecoder.toCSVString(
         new Object[]{userId,
             newMessages+processMessages,
             deliveredMessages,
             deliveredMessagesSMS,
             failedMessages+expiredMessages,
             failedMessagesSMS+expiredMessagesSMS}
      ));
    }
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

  public long getDeliveredMessagesSMS() {
    return deliveredMessagesSMS;
  }

  public long getFailedMessagesSMS() {
    return failedMessagesSMS;
  }

  public long getExpiredMessagesSMS() {
    return expiredMessagesSMS;
  }

  public String getUserDetails() {
    return user==null ? "" : user.getFirstName()+" "+user.getLastName();
  }
}
