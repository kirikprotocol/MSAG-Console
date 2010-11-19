package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.DeliveryInfo;
import mobi.eyeline.informer.admin.delivery.DeliveryStatistics;
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

  private User user;
  private DeliveryInfo info;
  private DeliveryStatistics stat;
  private Date startDate;
  private Date endDate;

  public MessagesByDeliveriesRecord(User user, DeliveryInfo info, DeliveryStatistics stat, Date startDate, Date endDate) {
    this.user = user;
    this.info = info;
    this.stat = stat;
    this.startDate = startDate;
    this.endDate = endDate;
  }

  public DeliveryInfo getInfo() {
    return info;
  }

  public DeliveryStatistics getStat() {
    return stat;
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
      writer.println(StringEncoderDecoder.toCSVString(info.getName(),
          info.getUserId(),
          stat.getDeliveryState().getStatus(),
          getStartDateString(),
          getEndDateString(),
          stat.getNewMessages(),
          stat.getProcessMessages(),
          stat.getDeliveredMessages(),
          stat.getFailedMessages(),
          stat.getExpiredMessages()));
    } else {
      writer.println(StringEncoderDecoder.toCSVString(info.getName(),
          info.getUserId(),
          stat.getDeliveryState().getStatus(),
          getStartDateString(),
          getEndDateString(),
          stat.getNewMessages() + stat.getProcessMessages(),
          stat.getDeliveredMessages(),
          stat.getFailedMessages() + stat.getExpiredMessages()));
    }
  }

  public Date getStartDate() {
    return startDate;
  }

  public Date getEndDate() {
    return endDate;
  }

  private String fmtDate(Date d) {
    if (d == null) return "";
    return new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").format(d);
  }

  public String getStartDateString() {
    return startDate == null ? "" : fmtDate(startDate);
  }

  public String getEndDateString() {
    return endDate == null ? "" : fmtDate(endDate);
  }

  public String getUserDetails() {
    return user == null ? "" : user.getFirstName() + " " + user.getLastName();
  }


  @Override
  Object getAggregationKey() {
    throw new NotImplementedException();  
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
                return mul * o1.getInfo().getName().compareTo(o2.getInfo().getName());
              } else if (sortOrder.getColumnId().equals("userId")) {
                return mul * o1.getInfo().getUserId().compareTo(o2.getInfo().getUserId());
              } else if (sortOrder.getColumnId().equals("status")) {
                return mul * o1.getStat().getDeliveryState().getStatus().compareTo(o2.getStat().getDeliveryState().getStatus());
              } else if (sortOrder.getColumnId().equals("new")) {
                return o1.getStat().getNewMessages() >= o2.getStat().getNewMessages() ? mul : -mul;
              } else if (sortOrder.getColumnId().equals("process")) {
                return o1.getStat().getProcessMessages() >= o2.getStat().getProcessMessages() ? mul : -mul;
              } else if (sortOrder.getColumnId().equals("delivered")) {
                return o1.getStat().getDeliveredMessages() >= o2.getStat().getDeliveredMessages() ? mul : -mul;
              } else if (sortOrder.getColumnId().equals("failed")) {
                return o1.getStat().getFailedMessages() >= o2.getStat().getFailedMessages() ? mul : -mul;
              } else if (sortOrder.getColumnId().equals("expired")) {
                return o1.getStat().getExpiredMessages() >= o2.getStat().getExpiredMessages() ? mul : -mul;
              } else if (sortOrder.getColumnId().equals("startDate")) {
                if (o1.getStartDate() == null) return (o2.getStartDate() == null ? 0 : mul);
                return mul * o1.getStartDate().compareTo(o2.getStartDate());
              } else if (sortOrder.getColumnId().equals("endDate")) {
                if (o1.getEndDate() == null) return (o2.getEndDate() == null ? 0 : mul);
                return mul * o1.getEndDate().compareTo(o2.getEndDate());
              } else if (sortOrder.getColumnId().equals("wait")) {
                return o1.getStat().getNewMessages() + o1.getStat().getProcessMessages() >= o2.getStat().getNewMessages() + o2.getStat().getProcessMessages() ? mul : -mul;
              } else if (sortOrder.getColumnId().equals("notdelivered")) {
                return o1.getStat().getFailedMessages() + o1.getStat().getExpiredMessages() >= o2.getStat().getFailedMessages() + o2.getStat().getExpiredMessages() ? mul : -mul;
              }
              return 0;
            }
    };
  }




}
