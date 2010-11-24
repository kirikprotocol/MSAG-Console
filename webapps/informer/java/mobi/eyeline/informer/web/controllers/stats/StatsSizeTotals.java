package mobi.eyeline.informer.web.controllers.stats;

import java.text.DecimalFormat;
import java.util.Calendar;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 24.11.2010
 * Time: 11:23:34
 */
public class StatsSizeTotals  implements DeliveryStatTotals  {

  private int count;
  private long size;
  private DecimalFormat fmt;

  public StatsSizeTotals() {
    reset();
    fmt = StatsSizeRecord.createSizeFormat();
  }

  public void reset() {
    count=0;
    size=0;
    //To change body of implemented methods use File | Settings | File Templates.
  }

  public void add(AggregatedRecord other) {
    StatsSizeRecord r = (StatsSizeRecord) other;
    count+=r.getCount();
    size+=r.getSize();

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

  public void setSize(long size) {
    this.size = size;
  }

  public String getSizeFormatted() {
    synchronized (fmt) {
      return fmt.format(size);
    }
  }
}
