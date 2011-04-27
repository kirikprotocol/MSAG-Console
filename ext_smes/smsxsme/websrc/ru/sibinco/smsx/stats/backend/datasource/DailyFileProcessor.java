package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;

import java.io.File;
import java.util.*;
import java.util.regex.Pattern;

/**
 * @author Aleksandr Khalitov
 */
class DailyFileProcessor {

  private final String one = "1";

  private static final Pattern commaPattern = Pattern.compile(",");

  private final FileStatsProcessor fileProcessor;

  private final FileStatsProcessor.StatsFileFilter fileFilter;

  private final ProgressListener progressListener;

  DailyFileProcessor(File artefactsDir, Date from, Date till, ProgressListener p) {
    this.fileProcessor = new FileStatsProcessor(artefactsDir, from, till);
    this.fileFilter = new FileStatsProcessor.StatsFileFilter("-traffic.csv", from, till);
    this.progressListener = p;
  }

  private final Set websmsSmes = new HashSet(){{
    add("websms");add("websyssms");
  }};

  protected final Collection process(ShutdownIndicator shutdownIndicator) throws StatisticsException {
    final Map daily = new HashMap(200);
    fileProcessor.visitFiles(fileFilter, createLineVisitor(daily, shutdownIndicator), new ProgressListener(){
      public void setProgress(int _progress) {
        progressListener.setProgress(3*_progress/4);
      }
    });
    try{
      return daily.values();
    }finally {
      progressListener.setProgress(100);
    }
  }

  FileStatsProcessor.LineVisitor createLineVisitor(final Map daily, final ShutdownIndicator shutdownIndicator) {
    return new FileStatsProcessor.LineVisitor() {
      private final Map dates = new HashMap(10);
      public void visit(String fileName, String line) throws StatisticsException {
        if(shutdownIndicator.isShutdown()) {
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
        WebDaily old = (WebDaily)daily.get(dailyKey);
        if(old == null) {
          daily.put(dailyKey, new WebDaily(dailyKey.date, dailyKey.region, dailyKey.msc, c));
        }else {
          old.incrementCount(c);
        }
      }
    };
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
