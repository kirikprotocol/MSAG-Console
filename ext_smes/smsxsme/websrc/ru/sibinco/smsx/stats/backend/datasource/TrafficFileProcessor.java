package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;

import java.io.File;
import java.util.*;
import java.util.regex.Pattern;

/**
 * @author Aleksandr Khalitov
 */
class TrafficFileProcessor {

  private final Set serviceIds;

  private final String one = "1";

  private final static Pattern commaPattern = Pattern.compile(",");

  private final FileStatsProcessor fileProcessor;

  private final FileStatsProcessor.StatsFileFilter fileFilter;

  private final ProgressListener progressListener;

  TrafficFileProcessor(File artefactsDir, Date from, Date till, ProgressListener p, Set serviceIds) {
    this.fileProcessor = new FileStatsProcessor(artefactsDir, from, till);
    this.serviceIds = serviceIds;
    this.fileFilter = new FileStatsProcessor.StatsFileFilter("-traffic.csv", from, till);
    this.progressListener = p;
  }

  protected final Collection process(ShutdownIndicator shutdownIndicator) throws StatisticsException {
    final Map traffic = new HashMap(200);
    fileProcessor.visitFiles(fileFilter, createLineVisitor(traffic, shutdownIndicator), new ProgressListener(){
      public void setProgress(int _progress) {
        progressListener.setProgress(3*_progress/4);
      }
    });
    try{
      return traffic.values();
    }finally {
      progressListener.setProgress(100);
    }
  }

  FileStatsProcessor.LineVisitor createLineVisitor(final Map traffic, final ShutdownIndicator shutdownIndicator) {
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
        Traffic old = (Traffic)traffic.get(key);
        if(old == null) {
          traffic.put(key, new Traffic(key.region, serviceId, key.msc, c));
        }else {
          old.incrementCount(c);
        }
      }
    };
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
