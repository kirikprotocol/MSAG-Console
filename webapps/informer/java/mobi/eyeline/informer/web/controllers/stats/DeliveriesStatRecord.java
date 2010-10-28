package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.DeliveryInfo;
import mobi.eyeline.informer.admin.delivery.DeliveryStatistics;
import mobi.eyeline.informer.util.StringEncoderDecoder;

import java.io.PrintWriter;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 28.10.2010
 * Time: 11:58:56
 */
public class DeliveriesStatRecord {

  private DeliveryInfo info;
  private DeliveryStatistics stat;

  public DeliveriesStatRecord(DeliveryInfo info, DeliveryStatistics stat) {
    this.info = info;
    this.stat = stat;
  }

  public DeliveryInfo getInfo() {
    return info;
  }

  public DeliveryStatistics getStat() {
    return stat;
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
      stat.getExpiredMessages()
         //todo dates
    }));
  }
}
