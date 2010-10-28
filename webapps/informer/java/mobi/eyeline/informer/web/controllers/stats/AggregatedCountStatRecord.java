package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.PrintWriter;
import java.util.Calendar;
import java.util.Comparator;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 16:22:41
 */
public class AggregatedCountStatRecord extends AggregatedStatRecord {

  private int counter;


  public AggregatedCountStatRecord(Calendar periodStart, AggregationType type, int counter, boolean isParent) {
    super(periodStart,type,isParent);
    this.counter = counter;
    if(getChildAggregationType()!=null) {
      AggregatedCountStatRecord child = new AggregatedCountStatRecord(periodStart,getChildAggregationType(),counter,false);
      addChild(child);
    }
  }



  public void add(AggregatedStatRecord r) {
    AggregatedCountStatRecord other = (AggregatedCountStatRecord) r;
    this.counter += other.counter;
    addChildren(r);
  }

  public int getCounter() {
    return counter;
  }


  void printCSV(PrintWriter writer) {
    if(isParent()) {
      writer.println(StringEncoderDecoder.toCSVString(new Object[]{getPeriodString(),"",getCounter()}));
    }
    else {
      writer.println(StringEncoderDecoder.toCSVString(new Object[]{"",getPeriodString(),getCounter()}));
    }
  }

  public Comparator getRecordsComparator(final DataTableSortOrder sortOrder) {

    return new Comparator<AggregatedCountStatRecord>() {
      public int compare(AggregatedCountStatRecord o1, AggregatedCountStatRecord o2) {
        final int mul = sortOrder.isAsc() ? 1 : -1;
        if (sortOrder.getColumnId().equals("period")) {
          return mul*o1.getStartCalendar().compareTo(o2.getStartCalendar());
        }
        else if (sortOrder.getColumnId().equals("counter")) {
          return o1.getCounter() >= o2.getCounter() ? mul : -mul;
        }        
        return 0;
      }
    };
  }

}