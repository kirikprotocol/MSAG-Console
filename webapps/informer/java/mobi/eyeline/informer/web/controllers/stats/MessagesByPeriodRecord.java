package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.DeliveryStatRecord;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.PrintWriter;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 16:22:41
 */
public class MessagesByPeriodRecord extends TimeAggregatedStatRecord {


  private long newMessages;
  private long processMessages;
  private long deliveredMessages;
  private long failedMessages;
  private long expiredMessages;
  private long deliveredMessagesSMS;
  private long failedMessagesSMS;
  private long expiredMessagesSMS;

  public MessagesByPeriodRecord(DeliveryStatRecord dsr, TimeAggregationType type, boolean isParent) {
    super(dsr.getDate(),type,isParent);
    this.newMessages = dsr.getNewmessages();
    this.processMessages = dsr.getProcessing();
    this.deliveredMessages = dsr.getDelivered();
    this.failedMessages = dsr.getFailed();
    this.expiredMessages = dsr.getExpired();
    this.deliveredMessagesSMS = dsr.getDeliveredSMS();
    this.failedMessagesSMS= dsr.getFailedSMS();
    this.expiredMessagesSMS= dsr.getExpiredSMS();

    if(getChildAggregationType()!=null) {
      MessagesByPeriodRecord child = new MessagesByPeriodRecord(dsr,getChildAggregationType(),false);
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
    addChildren(r);
  }




  void printCSV(PrintWriter writer, boolean detalized ) {
    if(detalized ) {
      if(isParent()) {
        writer.println(StringEncoderDecoder.toCSVString(new Object[]{getPeriodString(),"",
            getNewMessages(),
            getProcessMessages(),
            getDeliveredMessages(),getDeliveredMessagesSMS(),
            getFailedMessages(),getFailedMessagesSMS(),
            getExpiredMessages(),getExpiredMessagesSMS()
        }
        ));
      }
      else {
        writer.println(StringEncoderDecoder.toCSVString(new Object[]{"",getPeriodString(),
            getNewMessages(),
            getProcessMessages(),
            getDeliveredMessages(),getDeliveredMessagesSMS(),
            getFailedMessages(),getFailedMessagesSMS(),
            getExpiredMessages(),getExpiredMessagesSMS()
        }
        ));
      }
    }
    else {
      if(isParent()) {
        writer.println(StringEncoderDecoder.toCSVString(new Object[]{getPeriodString(),"",
            getNewMessages()+getProcessMessages(),
            getDeliveredMessages(),getDeliveredMessagesSMS(),
            getFailedMessages()+getExpiredMessages(),getFailedMessagesSMS()+getExpiredMessagesSMS(),
        }
        ));
      }
      else {
        writer.println(StringEncoderDecoder.toCSVString(new Object[]{"",getPeriodString(),
            getNewMessages()+getProcessMessages(),
            getDeliveredMessages(),getDeliveredMessagesSMS(),
            getFailedMessages()+getExpiredMessages(),getFailedMessagesSMS()+getExpiredMessagesSMS()
        }
        ));
      }
    }
  }

  @Override
  void printCSVheader(PrintWriter writer, boolean detalized) {
    if(detalized) {
      writer.println(StringEncoderDecoder.toCSVString(new Object[]{"PERIOD","",
            "NEW",
            "PROCESS",
            "DELIVERED","DELIVERED SMS",
            "FAILED","FAILED SMS",
            "EXPIRED","EXPIRED SMS"}
      ));
    }
    else {
    writer.println(StringEncoderDecoder.toCSVString(new Object[]{"PERIOD","",
          "WAIT",
          "DELIVERED","DELIVERED SMS",
          "NOT DELIVERED","NOT DELIVERED SMS",
          }
    ));
    }

  }


  public Comparator getRecordsComparator(final DataTableSortOrder sortOrder) {

    return new Comparator<MessagesByPeriodRecord>() {
      public int compare(MessagesByPeriodRecord o1, MessagesByPeriodRecord o2) {
        final int mul = sortOrder.isAsc() ? 1 : -1;
        if (sortOrder.getColumnId().equals("period")) {
          return mul*o1.getStartCalendar().compareTo(o2.getStartCalendar());
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
}