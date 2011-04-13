package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;
import ru.sibinco.smsx.stats.backend.Visitor;

import java.io.File;
import java.io.FilenameFilter;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class TrafficFileProcessor extends FileStatsProcessor{

  private final Set serviceIds;

  private final String one = "1";
  private final String zero = "0";

  private final Map traffic = new HashMap(2000);

  public TrafficFileProcessor(File artefactsDir, Date from, Date till, Progress p, Set serviceIds) {
    super(artefactsDir, from, till, p);
    this.serviceIds = serviceIds;
  }

  protected LineVisitor getLineVisitor() {
    return new LineVisitor() {
      private final Map dates = new HashMap(10);

      public void visit(String fileName, String line) throws StatisticsException {
        if(Thread.currentThread().isInterrupted()) {
          throw new StatisticsException(StatisticsException.Code.INTERRUPTED);
        }
        String date = (String)dates.get(fileName);
        if(date == null) {
          date = fileName.substring(0, fileName.indexOf('-'));
          dates.put(fileName, date);
        }
        String[] ss = commaPattern.split(line, 5);
        int serviceId = Integer.parseInt(ss[0]);
        if(serviceIds != null && !serviceIds.contains(new Integer(serviceId))) {
          return;
        }

        TrafficKey key = new TrafficKey(); //SERVICE_ID,MSC,REGION,SRC_SME_ID,COUNT
        key.serviceId = serviceId;
        key.msc = ss[1].equals(one);
        key.region = ss[2];
        int c = Integer.parseInt(ss[4]);
        Integer count = (Integer)traffic.get(key);
        traffic.put(key, new Integer(count == null ? c : count.intValue()+c));
      }
    };
  }

  protected FilenameFilter getFilenameFilter() {
    return new StatsFileFilter("-traffic.csv", from, till);
  }

  protected void getResults(Visitor v) throws StatisticsException {
    Iterator i = traffic.entrySet().iterator();
    while(i.hasNext()) {
      Map.Entry e = (Map.Entry)i.next();
      TrafficKey key = (TrafficKey)e.getKey();
      v.visit(new Traffic(key.region, key.serviceId, key.msc, ((Integer)e.getValue()).intValue()));
      i.remove();
    }
  }

  private static class TrafficKey {
    private String region;
    private boolean msc;
    private int serviceId;

  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;
    TrafficKey that = (TrafficKey) o;
    return msc == that.msc && serviceId == that.serviceId && !(region != null ? !region.equals(that.region) : that.region != null);
  }
  public int hashCode() {
    int result = region != null ? region.hashCode() : 0;
    result = 31 * result + serviceId;
    result = 31 * result + (msc ? 1 : 0);
    return result;
  }
  }
}
