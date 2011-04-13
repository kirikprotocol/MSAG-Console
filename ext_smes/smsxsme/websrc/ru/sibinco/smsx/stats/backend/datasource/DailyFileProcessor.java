package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;
import ru.sibinco.smsx.stats.backend.Visitor;

import java.io.File;
import java.io.FilenameFilter;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
class DailyFileProcessor extends FileStatsProcessor{

  private final String one = "1";

  private final Map daily = new HashMap(2000);

  DailyFileProcessor(File artefactsDir, Date from, Date till, Progress p) {
    super(artefactsDir, from, till, p);
  }

  private final Set websmsSmes = new HashSet(){{
    add("websms");add("websyssms");
  }};

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
        String[] ss = commaPattern.split(line, 5); //SERVICE_ID,MSC,REGION,SRC_SME_ID,COUNT
        if(!websmsSmes.contains(ss[3])) {
          return;
        }
        DailyKey dailyKey = new DailyKey();
        dailyKey.msc = ss[1].equals(one);
        dailyKey.region = ss[2];
        dailyKey.date = date;
        int c = Integer.parseInt(ss[4]);
        Integer count = (Integer)daily.get(dailyKey);
        daily.put(dailyKey, new Integer(count == null ? c : count.intValue() + c));
      }
    };
  }

  protected FilenameFilter getFilenameFilter() {
    return new StatsFileFilter("-traffic.csv", from, till);
  }

  protected void getResults(Visitor v) throws StatisticsException{
    Iterator i = daily.entrySet().iterator();
    while(i.hasNext()) {
      Map.Entry e = (Map.Entry)i.next();
      DailyKey key = (DailyKey)e.getKey();
      if(!v.visit(new WebDaily(key.date, key.region, key.msc, ((Integer)e.getValue()).intValue()))) {
        return;
      }
      i.remove();
    }
  }

  private static class DailyKey {
    private String region;
    private boolean msc;
    private String date;

    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;
      DailyKey dailyKey = (DailyKey) o;
      return msc == dailyKey.msc && !(date != null ? !date.equals(dailyKey.date) : dailyKey.date != null) &&
          !(region != null ? !region.equals(dailyKey.region) : dailyKey.region != null);
    }
    public int hashCode() {
      int result = region != null ? region.hashCode() : 0;
      result = 31 * result + (msc ? 1 : 0);
      result = 31 * result + (date != null ? date.hashCode() : 0);
      return result;
    }
  }

}
