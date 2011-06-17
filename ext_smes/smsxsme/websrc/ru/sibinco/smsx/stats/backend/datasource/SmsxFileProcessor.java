package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;

import java.io.File;
import java.util.*;
import java.util.regex.Pattern;

/**
 * @author Aleksandr Khalitov
 */

class SmsxFileProcessor {

  private static final Pattern commaPattern = Pattern.compile(",");

  private final Set serviceIds;

  private final ProgressListener progressListener;

  private final FileStatsProcessor fileProcessor;

  private final FileStatsProcessor.StatsFileFilter filter;

  SmsxFileProcessor(File artefactsDir, Date from, Date till, ProgressListener p, Set serviceIds) {
    this.fileProcessor = new FileStatsProcessor(artefactsDir, from, till);
    this.serviceIds = serviceIds;
    this.progressListener = p;
    this.filter = new FileStatsProcessor.StatsFileFilter("-smsx-users.csv", from, till);
  }


  protected final Collection process(ShutdownIndicator shutdownIndicator) throws StatisticsException {
    final Map smsxUsers = new HashMap(1000);
    fileProcessor.visitFiles(filter, createLineVisitor(smsxUsers, shutdownIndicator), new ProgressListener(){
      public void setProgress(int _progress) {
        progressListener.setProgress(3*_progress/4);
      }
    });
    try{
      return smsxUsers.values();
    }finally {
      progressListener.setProgress(100);
    }
  }

  FileStatsProcessor.LineVisitor createLineVisitor(final Map smsxUsers, final ShutdownIndicator shutdownIndicator) {
    return new  FileStatsProcessor.LineVisitor() {
      public void visit(String fileName, String line) throws StatisticsException {
        if(shutdownIndicator.isShutdown()) {
          throw new StatisticsException(StatisticsException.Code.INTERRUPTED);
        }
        String[] ss = commaPattern.split(line, 3);     //SERVICE_ID,SRC_ADDRESS,REGION
        int serviceId = Integer.parseInt(ss[0]);
        if(serviceIds != null && !serviceIds.contains(new Integer(serviceId))) {
          return;
        }
        SmsxUserKey key = new SmsxUserKey();
        key.region = ss[2];
        key.service_id = serviceId;
        SmsxUsers old = (SmsxUsers)smsxUsers.get(key);
        if(old == null) {
          old = new SmsxUsers(key.service_id, key.region,0);
          smsxUsers.put(key, old);
        }
        old.addMsisdn(ss[1]);
      }
    };
  }

  private class SmsxUserKey {
    String region;
    int service_id;

    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;

      SmsxUserKey that = (SmsxUserKey) o;

      return service_id == that.service_id && (region != null ? region.equals(that.region) : that.region == null);
    }

    public int hashCode() {
      int result = region != null ? region.hashCode() : 0;
      result = 31 * result + service_id;
      return result;
    }
  }
}
