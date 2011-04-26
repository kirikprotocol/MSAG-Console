package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;
import ru.sibinco.smsx.stats.backend.Visitor;

import java.io.File;
import java.io.FilenameFilter;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
//todo По-моему, наследование в данном случае - не лучшее решение. Ты его используешь исключительно для того, чтобы
//todo переиспользовать функциональность класса FileStatsProcessor. Лучше прибегнуть к делегированию.
class WebRegionsStatsProcessor extends FileStatsProcessor {

  private final String one = "1";
  private final String zero = "0";

  private final Map webSmsUsers = new HashMap(200);

  WebRegionsStatsProcessor(File artefactsDir, Date from, Date till, Progress p) {
    super(artefactsDir, from, till, p);
  }

  protected LineVisitor getLineVisitor() {
    return new LineVisitor() {
      public void visit(String fileName, String line) throws StatisticsException {
        if(Thread.currentThread().isInterrupted()) {
          throw new StatisticsException(StatisticsException.Code.INTERRUPTED);
        }
        String[] ss = commaPattern.split(line, 4);   //INDEX,MSC,ADDRESS,REGION
        WebSmsKey key = new WebSmsKey();
        String index = ss[0];
        String msc = ss[1];
        key.msc = msc.equals(one);
        key.region = ss[3];
        WebSmsValue count = (WebSmsValue)webSmsUsers.get(key);
        if(count == null) {
          count = new WebSmsValue();
          webSmsUsers.put(key, count);
        }
        if(index.equals(one)) {
          count.src++;
        }else {
          count.dst++;
        }
      }

    };
  }

  protected FilenameFilter getFilenameFilter() {
    return new StatsFileFilter("-websms-users.csv", from, till);
  }

  protected void getResults(Visitor v) throws StatisticsException{

    Iterator i = webSmsUsers.entrySet().iterator();
    while(i.hasNext()) {
      Map.Entry e = (Map.Entry)i.next();
      WebSmsKey key = (WebSmsKey)e.getKey();
      WebSmsValue value = (WebSmsValue)e.getValue();
      v.visit(new WebRegion(key.region, key.msc, value.src, value.dst));
      i.remove();
    }
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

  private class WebSmsValue {
    private int src;
    private int dst;
  }
}
