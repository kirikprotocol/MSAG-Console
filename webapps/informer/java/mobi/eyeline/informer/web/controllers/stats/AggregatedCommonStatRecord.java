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
public class AggregatedCommonStatRecord extends AggregatedStatRecord {

  private int delivered;
  private int failed;

  public AggregatedCommonStatRecord(DeliveryStatRecord dsr, AggregationType type, boolean isParent) {
    super(dsr.getYear(),dsr.getMonth(),dsr.getDay(),dsr.getHour(),dsr.getMinute(),type,isParent);
    this.delivered = dsr.getDelivered();
    this.failed = dsr.getFailed();
    if(getChildAggregationType()!=null) {
      AggregatedCommonStatRecord child = new AggregatedCommonStatRecord(dsr,getChildAggregationType(),false);
      addChild(child);
    }
  }



  void add(AggregatedStatRecord r) {
    AggregatedCommonStatRecord other = (AggregatedCommonStatRecord) r;
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

    return new Comparator<AggregatedCommonStatRecord>() {
      public int compare(AggregatedCommonStatRecord o1, AggregatedCommonStatRecord o2) {
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
