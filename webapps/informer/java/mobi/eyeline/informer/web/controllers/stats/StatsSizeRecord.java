package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.PrintWriter;
import java.text.DateFormat;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Comparator;
import java.util.Locale;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 24.11.2010
 * Time: 11:25:04
 */
public class StatsSizeRecord extends TimeAggregatedStatRecord {
  private int count;
  private long size;
  private DecimalFormat fmt;
  private static DateFormat idformat = new SimpleDateFormat("yyyyMMddHHmm");

  public StatsSizeRecord(Calendar periodStart, AggregationType type, boolean isParent, int count, long size) {
    super(periodStart, type, isParent);
    this.count = count;
    this.size = size;
    if (getChildAggregationType() != null) {
      StatsSizeRecord child = new StatsSizeRecord(periodStart, getChildAggregationType(), false,count, size);
      addChild(child);
    }
    fmt = createSizeFormat();

  }

  public static DecimalFormat createSizeFormat() {
    DecimalFormatSymbols symbols = new DecimalFormatSymbols(Locale.US);
    symbols.setGroupingSeparator(',');
    symbols.setDecimalSeparator('.');

    DecimalFormat fmt = new DecimalFormat();
    fmt.setGroupingSize(3);
    fmt.setGroupingUsed(true);
    fmt.setDecimalFormatSymbols(symbols);
    fmt.setMaximumFractionDigits(0);
    fmt.setMinimumIntegerDigits(1);
    return fmt;
  }


  @Override
  public void add(AggregatedRecord other) {
    StatsSizeRecord r = (StatsSizeRecord) other;
    count+=r.count;
    size +=r.size;
    addChildren(r);
  }

  @Override
  public Comparator getRecordsComparator(final DataTableSortOrder sortOrder) {
    return new Comparator<StatsSizeRecord>() {

                public int compare(StatsSizeRecord o1, StatsSizeRecord o2) {

                  final int mul = sortOrder.isAsc() ? 1 : -1;
                  if (sortOrder.getColumnId().equals("date")) {
                    return mul * o1.getStartCalendar().compareTo(o2.getStartCalendar());
                  }
                  if (sortOrder.getColumnId().equals("count")) {
                    return o1.count >= o2.count ? mul : -mul;
                  }
                  if (sortOrder.getColumnId().equals("size")) {
                    return o1.size >= o2.size ? mul : -mul;
                  }
                  return 0;
                }
        };

  }

  @Override
  public void printCSVheader(PrintWriter writer, boolean detalised) {
    writer.println(StringEncoderDecoder.toCSVString(';',"DATE","","FILES","SIZE"));
  }

  @Override
  void printCSV(PrintWriter writer, boolean detalized) {
    if (isParent()) {
      writer.println(StringEncoderDecoder.toCSVString(';',getPeriodString(), "", count,size));
    } else {
      writer.println(StringEncoderDecoder.toCSVString(';',"", getPeriodString(), count,size));
    }
  }

  public int getCount() {
    return count;
  }

  public void setCount(int count) {
    this.count = count;
  }

  public long getSize() {
    return size;
  }

  public String getSizeFormatted() {
    synchronized (fmt) {
      return fmt.format(size);
    }
  }

  public void setSize(long size) {
    this.size = size;
  }

  public String getPeriodId() {
    synchronized (idformat) {
      return idformat.format(startCalendar.getTime())+"-"+idformat.format(endCalendar.getTime());
    }
  }

  public static DateFormat getPeriodIdFormat() {
    return (DateFormat) idformat.clone();
  }

}
