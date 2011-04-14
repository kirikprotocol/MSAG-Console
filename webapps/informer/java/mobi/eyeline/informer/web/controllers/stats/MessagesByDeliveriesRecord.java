package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatRecord;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.Comparator;
import java.util.Date;
import java.util.TreeMap;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 28.10.2010
 * Time: 11:58:56
 */
public class MessagesByDeliveriesRecord extends AggregatedRecord {

  private String login;
  private long deliveryId;
  private long newMessages;
  private long procMessages;
  private long deliveredMessages;
  private long deliveredSms;
  private long failedMessages;
  private long failedSms;
  private long expiredMessages;
  private long expiredSms;

  private final User user;

  private Date minDate;
  private Date maxDate;

  private String region;
  private Integer regionId;
  private final boolean deletedRegion;

  private final boolean deletedDelviery;
  private final String deliveryName;

  private String deliveryStatus;

  public MessagesByDeliveriesRecord(DeliveryStatRecord rec, User user, String deliveryName, boolean deletedDelivery, String region, boolean deletedRegion, boolean isParent) {
    this.login = rec.getUser();
    this.deliveryId = rec.getTaskId();
    this.isParent = isParent;
    this.region = region;
    this.regionId = rec.getRegionId();
    this.deletedRegion = deletedRegion;
    this.deletedDelviery = deletedDelivery;
    this.deliveryName =  deliveryName;
    this.user = user;
    incNewMessages(rec.getNewmessages());
    incProcMessages(rec.getProcessing());
    incDeliveredMessages(rec.getDelivered());
    incDeliveredSms(rec.getDeliveredSMS());
    incFailedMessages(rec.getFailed());
    incFailedSms(rec.getFailedSMS());
    incExpiredMessages(rec.getExpired());
    incExpiredSms(rec.getExpiredSMS());
    if(isParent) {
      innerRowsMap = new TreeMap<Object, AggregatedRecord>();
      MessagesByDeliveriesRecord c = new MessagesByDeliveriesRecord(rec, user, deliveryName, deletedDelivery, region, deletedRegion, false);
      c.setRegion(region);
      addChild(c);
    }
  }

  public String getDeliveryStatus() {
    return deliveryStatus;
  }

  public void setDeliveryStatus(String deliveryStatus) {
    this.deliveryStatus = deliveryStatus;
  }

  public boolean isDeletedRegion() {
    return deletedRegion;
  }

  public boolean isDeletedDelviery() {
    return deletedDelviery;
  }

  public Integer getRegionId() {
    return regionId;
  }

  public void setRegionId(Integer regionId) {
    this.regionId = regionId;
  }

  public String getRegion() {
    return region;
  }

  public void setRegion(String region) {
    this.region = region;
  }

  public String getLogin() {
    return login;
  }

  public void setLogin(String login) {
    this.login = login;
  }

  public long getDeliveryId() {
    return deliveryId;
  }

  public void setDeliveryId(long deliveryId) {
    this.deliveryId = deliveryId;
  }

  public long getNewMessages() {
    return newMessages;
  }

  public void setNewMessages(long newMessages) {
    this.newMessages = newMessages;
  }

  public void incNewMessages(long newMessages) {
    this.newMessages += newMessages;
  }

  public long getProcMessages() {
    return procMessages;
  }

  public void setProcMessages(long procMessages) {
    this.procMessages = procMessages;
  }

  public void incProcMessages(long procMessages) {
    this.procMessages += procMessages;
  }

  public long getDeliveredMessages() {
    return deliveredMessages;
  }

  public void setDeliveredMessages(long deliveredMessages) {
    this.deliveredMessages = deliveredMessages;
  }

  public void incDeliveredMessages(long deliveredMessages) {
    this.deliveredMessages += deliveredMessages;
  }

  public long getDeliveredSms() {
    return deliveredSms;
  }

  public void setDeliveredSms(long deliveredSms) {
    this.deliveredSms = deliveredSms;
  }

  public void incDeliveredSms(long deliveredSms) {
    this.deliveredSms += deliveredSms;
  }

  public long getFailedMessages() {
    return failedMessages;
  }

  public void setFailedMessages(long failedMessages) {
    this.failedMessages = failedMessages;
  }

  public void incFailedMessages(long failedMessages) {
    this.failedMessages += failedMessages;
  }

  public long getFailedSms() {
    return failedSms;
  }

  public void setFailedSms(long failedSms) {
    this.failedSms = failedSms;
  }

  public void incFailedSms(long failedSms) {
    this.failedSms += failedSms;
  }

  public long getExpiredMessages() {
    return expiredMessages;
  }

  public void setExpiredMessages(long expiredMessages) {
    this.expiredMessages = expiredMessages;
  }

  public void incExpiredMessages(long expiredMessages) {
    this.expiredMessages += expiredMessages;
  }

  public long getExpiredSms() {
    return expiredSms;
  }

  public void setExpiredSms(long expiredSms) {
    this.expiredSms = expiredSms;
  }

  public void incExpiredSms(long expiredSms) {
    this.expiredSms += expiredSms;
  }

  public void updateTime(Date date) {
    if (minDate == null || minDate.after(date))
      minDate = date;
    if (maxDate == null || maxDate.before(date))
      maxDate = date;
  }

  public Date getMinDate() {
    return minDate;
  }

  public Date getMaxDate() {
    return maxDate;
  }

  public User getUser() {
    return user;
  }

  public void printCSVheader(PrintWriter writer, boolean fullMode) {
    if (fullMode) {
      writer.println(StringEncoderDecoder.toCSVString(';',"NAME", "REGION",
          "USER",
          "STATUS",
          "STARTDATE",
          "ENDDATE",
          "NEW",
          "PROCESS",
          "DELIVERED",
          "FAILED",
          "EXPIRED"));
    } else {
      writer.println(StringEncoderDecoder.toCSVString(';',"NAME", "REGION",
          "USER",
          "STATUS",
          "STARTDATE",
          "ENDDATE",
          "WAIT",
          "DELIVERED",
          "NOTDELIVERED"));

    }
  }

  public void printWithChildrenToCSV(PrintWriter writer, boolean fullMode) {

    for(AggregatedRecord r1 : innerRowsMap.values()) {
      MessagesByDeliveriesRecord r = (MessagesByDeliveriesRecord) r1;
      if (fullMode) {
        writer.println(StringEncoderDecoder.toCSVString(';',r.deliveryName, r.region,
            r.login,
            deliveryStatus,
            fmtDate(minDate),
            fmtDate(maxDate),
            r.newMessages,
            r.procMessages,
            r.deliveredMessages,
            r.failedMessages,
            r.expiredMessages));
      } else {
        writer.println(StringEncoderDecoder.toCSVString(';',r.deliveryName, r.region,
            r.login,
            deliveryStatus,
            fmtDate(minDate),
            fmtDate(maxDate),
            r.newMessages,
            r.deliveredMessages,
            r.failedMessages + r.expiredMessages));
      }
    }
  }

  private String fmtDate(Date d) {
    if (d == null) return "";
    return new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").format(d);
  }

  public String getUserDetails() {
    return user == null ? "" : user.getFirstName() + " " + user.getLastName();
  }

  public String getDeliveryName() {
    return deliveryName;
  }

  @Override
  Object getAggregationKey() {
    if(isParent) {
      return deliveryId;
    }else {
      return regionId;
    }
  }

  @Override
  void add(AggregatedRecord r) {
    MessagesByDeliveriesRecord other = (MessagesByDeliveriesRecord)r;
    this.newMessages += other.getNewMessages();
    this.procMessages += other.getProcMessages();
    this.deliveredMessages += other.getDeliveredMessages();
    this.failedMessages += other.getFailedMessages();
    this.expiredMessages += other.getExpiredMessages();
    this.deliveredSms += other.getDeliveredSms();
    this.failedSms += other.getFailedSms();
    this.expiredSms += other.getExpiredSms();
    if(isParent) {
      addChild(r);
    }
  }

  Comparator getRecordsComparator(final DataTableSortOrder sortOrder) {
    return new Comparator<MessagesByDeliveriesRecord>() {

      public int compare(MessagesByDeliveriesRecord o1, MessagesByDeliveriesRecord o2) {

        final int mul = sortOrder.isAsc() ? 1 : -1;
        if (sortOrder.getColumnId().equals("name")) {
          return mul * o1.getDeliveryName().compareTo(o2.getDeliveryName());
        } else if (sortOrder.getColumnId().equals("userId")) {
          return mul * o1.getLogin().compareTo(o2.getLogin());
        } else if (sortOrder.getColumnId().equals("new")) {
          return o1.getNewMessages() >= o2.getNewMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("process")) {
          return o1.getProcMessages() >= o2.getProcMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("delivered")) {
          return o1.getDeliveredMessages() >= o2.getDeliveredMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("failed")) {
          return o1.getFailedMessages() >= o2.getFailedMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("expired")) {
          return o1.getExpiredMessages() >= o2.getExpiredMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("startDate")) {
        } else if (sortOrder.getColumnId().equals("wait")) {
          return o1.getNewMessages() + o1.getProcMessages() >= o2.getNewMessages() + o2.getProcMessages() ? mul : -mul;
        } else if (sortOrder.getColumnId().equals("notdelivered")) {
          return o1.getFailedMessages() + o1.getExpiredMessages() >= o2.getFailedMessages() + o2.getExpiredMessages() ? mul : -mul;
        }else if (sortOrder.getColumnId().equals("region")) {
          if(o1.getRegion() == null) {
            if(o2.getRegion() == null) {
              return 0;
            }
            return -mul;
          }else {
            if(o2.getRegion() == null) {
              return mul;
            }
            return mul * o1.getRegion().compareTo(o2.getRegion());
          }
        }
        return 0;
      }
    };
  }
}
