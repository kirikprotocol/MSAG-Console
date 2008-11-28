import mobi.eyeline.smsquiz.distribution.impl.*;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.Distribution;
import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.storage.ResultSet;
import org.apache.log4j.Logger;

import java.util.Date;
import java.util.StringTokenizer;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.FileNotFoundException;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;

/**
 * author: alkhal
 */
public class TestDistributionManager implements DistributionManager {

  private static int id = 0;
  private static final Logger logger = Logger.getLogger(DistributionInfoSmeManager.class);

  public String createDistribution(final Distribution distr, final Runnable task, String errorFile) throws DistributionException {
    id++;
    new Thread() {
      public void run() {
        task.run();
        BufferedReader reader = null;
        try{
          reader = new BufferedReader(new FileReader(distr.getFilePath()));
          String line;
          while((line = reader.readLine())!=null) {
            StringTokenizer tokenizer = new StringTokenizer(line,"|");
            Main.send(tokenizer.nextToken(),"170",tokenizer.nextToken());
          }
          System.out.println("All questions sent");
        } catch (IOException e) {
          e.printStackTrace();
        } finally {
          try {
            if(reader!=null) {
              reader.close();
            }
          } catch (IOException e) {
            e.printStackTrace();
          }
        }
      }
    }.start();
    return Integer.toString(id);
  }

  public String createDistribution(Distribution distr, Runnable task) throws DistributionException {
    return null;  //To change body of implemented methods use File | Settings | File Templates.
  }

  public ResultSet getStatistics(String id, Date startDate, Date endDate) throws DistributionException {
    return null;
  }

  public String repairStatus(String id, Runnable task, Distribution distribution) throws DistributionException {
    return null;  //To change body of implemented methods use File | Settings | File Templates.
  }

  public void resend(String str1, String str2) throws DistributionException{
    
  }

  public void removeDistribution(String taskId) throws DistributionException {
    //To change body of implemented methods use File | Settings | File Templates.
  }

  public String repairStatus(String id, String errorFile, Runnable task, Distribution distribution) throws DistributionException {
    return null;
  }

  public AbstractDynamicMBean getMonitor() {
    return null;
  }

  public void shutdown() {
    //To change body of implemented methods use File | Settings | File Templates.
  }

}
