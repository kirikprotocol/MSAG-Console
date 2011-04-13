package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;
import ru.sibinco.smsx.stats.backend.Visitor;

import java.io.File;
import java.io.FilenameFilter;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
class SmsxFileProcessor extends FileStatsProcessor {

  private final Set serviceIds;

  private  final Map smsxUsers = new HashMap(1000);

  SmsxFileProcessor(File artefactsDir, Date from, Date till, Progress p, Set serviceIds) {
    super(artefactsDir, from, till, p);
    this.serviceIds = serviceIds;
  }

  protected void getResults(Visitor v) throws StatisticsException {
    Iterator i = smsxUsers.entrySet().iterator();
    while(i.hasNext()) {
      Map.Entry e = (Map.Entry)i.next();
      SmsxUserKey key = (SmsxUserKey)e.getKey();
      v.visit(new SmsxUsers(key.service_id, key.region, ((Integer)e.getValue()).intValue()));
      i.remove();
    }
  }

  protected LineVisitor getLineVisitor() {
    return new LineVisitor() {
      public void visit(String fileName, String line) throws StatisticsException {
        if(Thread.currentThread().isInterrupted()) {
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
        Integer count = (Integer)smsxUsers.get(key);
        smsxUsers.put(key, new Integer(count == null ?  1 : count.intValue()+1));
      }

    };
  }

  protected FilenameFilter getFilenameFilter() {
    return new StatsFileFilter("-smsx-users.csv", from, till);
  }

  private class SmsxUserKey {
    String region;
    int service_id;

    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;

      SmsxUserKey that = (SmsxUserKey) o;

      return service_id == that.service_id && !(region != null ? !region.equals(that.region) : that.region != null);

    }

    public int hashCode() {
      int result = region != null ? region.hashCode() : 0;
      result = 31 * result + service_id;
      return result;
    }
  }
}
