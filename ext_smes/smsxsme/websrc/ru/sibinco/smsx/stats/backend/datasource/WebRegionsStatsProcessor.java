package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;

import java.io.File;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

/**
 * @author Aleksandr Khalitov
 */

class WebRegionsStatsProcessor {

  private final String one = "1";

  private static final Pattern commaPattern = Pattern.compile(",");

  private final FileStatsProcessor fileProcessor;

  private final FileStatsProcessor.StatsFileFilter statsFileFilter;

  private final ProgressListener progressListener;

  WebRegionsStatsProcessor(File artefactsDir, Date from, Date till, ProgressListener p) {
    fileProcessor = new FileStatsProcessor(artefactsDir, from, till);
    statsFileFilter = new FileStatsProcessor.StatsFileFilter("-websms-users.csv", from, till);
    this.progressListener = p;
  }

  protected final Collection process(ShutdownIndicator shutdownIndicator) throws StatisticsException {
    final Map regions = new HashMap(200);
    fileProcessor.visitFiles(statsFileFilter, createLineVisitor(regions, shutdownIndicator), new ProgressListener(){
      public void setProgress(int _progress) {
        progressListener.setProgress(3*_progress/4);
      }
    });
    try{
      return regions.values();
    }finally {
      progressListener.setProgress(100);
    }
  }

  FileStatsProcessor.LineVisitor createLineVisitor(final Map regions, final ShutdownIndicator shutdownIndicator) {
    return new FileStatsProcessor.LineVisitor() {
      public void visit(String fileName, String line) throws StatisticsException {
        if(shutdownIndicator.isShutdown()) {
          throw new StatisticsException(StatisticsException.Code.INTERRUPTED);
        }
        String[] ss = commaPattern.split(line, 4);   //INDEX,MSC,ADDRESS,REGION
        WebSmsKey key = new WebSmsKey();
        String index = ss[0];
        String msc = ss[1];
        key.msc = msc.equals(one);
        key.region = ss[3];
        WebRegion old = (WebRegion)regions.get(key);
        if(old == null) {
          old = new WebRegion(key.region, key.msc);
          regions.put(key, old);
        }
        if(index.equals(one)) {
          old.incrementSrcCount();
        }else {
          old.incrementDstCount();
        }
      }
    };
  }

  private class WebSmsKey {
    String region;
    boolean msc;

    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;

      WebSmsKey webSmsKey = (WebSmsKey) o;

      if (msc != webSmsKey.msc) return false;
      if (region != null ? !region.equals(webSmsKey.region) : webSmsKey.region != null) return false;

      return true;
    }

    public int hashCode() {
      int result = region != null ? region.hashCode() : 0;
      result = 31 * result + (msc ? 1 : 0);
      return result;
    }
  }
}
