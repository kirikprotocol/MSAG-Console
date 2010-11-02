package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.DeliveryInfo;
import mobi.eyeline.informer.admin.delivery.DeliveryStatistics;
import mobi.eyeline.informer.util.StringEncoderDecoder;

import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 28.10.2010
 * Time: 11:58:56
 */
public class MessagesByDeliveriesRecord {

  private DeliveryInfo info;
  private DeliveryStatistics stat;
  private Date startDate;
  private Date endDate;

  public MessagesByDeliveriesRecord(DeliveryInfo info, DeliveryStatistics stat, Date startDate, Date endDate) {
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

  public void printCSVHeader(PrintWriter writer) {
    writer.println(StringEncoderDecoder.toCSVString(new Object[]{
    "NAME",
    "USER",
    "STATUS",
    "NEW",
    "PROCESS",
    "DELIVERED",
    "FAILED",
    "EXPIRED",
    "STARTDATE",
    "ENDDATE"}));
  }

  public void printCSV(PrintWriter writer) {
     writer.println(StringEncoderDecoder.toCSVString(new Object[]{
      info.getName(),
      info.getUserId(),
      stat.getDeliveryState().getStatus(),
      stat.getNewMessages(),
      stat.getProcessMessages(),
      stat.getDeliveredMessages(),
      stat.getFailedMessages(),
      stat.getExpiredMessages(),
      getStartDateString(),
      getEndtDateString()
    }));
  }

  public Date getStartDate() {
    return startDate;
  }

  public Date getEndDate() {
    return endDate;
  }

  private String fmtDate(Date d) {
    if(d==null) return "";
    return new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").format(d);
  }

  public String getStartDateString() {
    return startDate==null ? "": fmtDate(startDate);
  }

  public String getEndtDateString() {
    return endDate==null ? "": fmtDate(endDate);
  }
}
