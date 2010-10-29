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

  private int delivered;
  private int failed;

  public MessagesByPeriodRecord(DeliveryStatRecord dsr, TimeAggregationType type, boolean isParent) {
    super(dsr.getDate(),type,isParent);
    this.delivered = dsr.getDelivered();
    this.failed = dsr.getFailed();
    if(getChildAggregationType()!=null) {
      MessagesByPeriodRecord child = new MessagesByPeriodRecord(dsr,getChildAggregationType(),false);
      addChild(child);
    }
  }



  public void add(AggregatedRecord r) {
    MessagesByPeriodRecord other = (MessagesByPeriodRecord) r;
    this.delivered += other.delivered;
    this.failed += other.failed;
    addChildren(r);
  }


  public int getDelivered() {
    return delivered;
  }

  public int getFailed() {
    return failed;
  }

  void printCSV(PrintWriter writer) {
    if(isParent()) {
      writer.println(StringEncoderDecoder.toCSVString(new Object[]{getPeriodString(),"",getDelivered(),getFailed()}));
    }
    else {
      writer.println(StringEncoderDecoder.toCSVString(new Object[]{"",getPeriodString(),getDelivered(),getFailed()}));
    }
  }


  public Comparator getRecordsComparator(final DataTableSortOrder sortOrder) {

    return new Comparator<MessagesByPeriodRecord>() {
      public int compare(MessagesByPeriodRecord o1, MessagesByPeriodRecord o2) {
        final int mul = sortOrder.isAsc() ? 1 : -1;
        if (sortOrder.getColumnId().equals("period")) {
          return mul*o1.getStartCalendar().compareTo(o2.getStartCalendar());
        }
        else if (sortOrder.getColumnId().equals("delivered")) {
          return o1.getDelivered() >= o2.getDelivered() ? mul : -mul;
        }
        else if (sortOrder.getColumnId().equals("failed")) {
          return o1.getFailed() >= o2.getFailed() ? mul : -mul;
        }
        return 0;
      }
    };
  }

}
