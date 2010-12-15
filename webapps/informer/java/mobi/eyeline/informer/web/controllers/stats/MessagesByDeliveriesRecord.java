package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import org.apache.commons.lang.NotImplementedException;

import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.Comparator;
import java.util.Date;

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

  private Delivery delivery;
  private User user;

  public MessagesByDeliveriesRecord(String login, long deliveryId) {
    this.login = login;
    this.deliveryId = deliveryId;
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

  public Delivery getDelivery() {
    return delivery;
  }

  public void setDelivery(Delivery delivery) {
    this.delivery = delivery;
  }

  public User getUser() {
    return user;
  }

  public void setUser(User user) {
    this.user = user;
  }

  public void printCSVheader(PrintWriter writer, boolean fullMode) {
    if (fullMode) {
      writer.println(StringEncoderDecoder.toCSVString("NAME",
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
      writer.println(StringEncoderDecoder.toCSVString("NAME",
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
    if (fullMode) {
      writer.println(StringEncoderDecoder.toCSVString(delivery.getName(),
          login,
          newMessages,
          procMessages,
          deliveredMessages,
          failedMessages,
          expiredMessages));
    } else {
      writer.println(StringEncoderDecoder.toCSVString(delivery.getName(),
          login,
          newMessages,
          deliveredMessages,
          failedMessages + expiredMessages));
    }
  }

  private String fmtDate(Date d) {
    if (d == null) return "";
    return new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").format(d);
  }

  public String getUserDetails() {
    return login == null ? "" : user.getFirstName() + " " + user.getLastName();
  }


  @Override
  Object getAggregationKey() {
    return deliveryId;
  }

  @Override
  void add(AggregatedRecord other) {
    throw new NotImplementedException();
  }

  Comparator getRecordsComparator(final DataTableSortOrder sortOrder) {
    return new Comparator<MessagesByDeliveriesRecord>() {

      public int compare(MessagesByDeliveriesRecord o1, MessagesByDeliveriesRecord o2) {

        final int mul = sortOrder.isAsc() ? 1 : -1;
        if (sortOrder.getColumnId().equals("name")) {
          return mul * o1.getDelivery().getName().compareTo(o2.getDelivery().getName());
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
        }
        return 0;
      }
    };
  }




}
