package mobi.eyeline.smsquiz.distribution;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;
import mobi.eyeline.smsquiz.storage.ResultSet;

import java.util.Date;

public interface DistributionManager {

  public String createDistribution(Distribution distr, Runnable task) throws DistributionException;

  public ResultSet getStatistics(String id, Date startDate, Date endDate) throws DistributionException;

  public String repairStatus(String id, Runnable task, Distribution distribution) throws DistributionException;

  public void resend(String msisdn, String taskId) throws DistributionException;

  public void removeDistribution(String distrId) throws DistributionException;

  public AbstractDynamicMBean getMonitor();

  public void shutdown();

}
 
