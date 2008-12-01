package mobi.eyeline.smsquiz.distribution;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;
import mobi.eyeline.smsquiz.storage.ResultSet;

import java.util.Date;

public interface DistributionManager {

  public static enum State {GENERATED, UNGENERATED, ERROR}

  public String createDistribution(Distribution distr) throws DistributionException;

  public void removeDistribution(String distrId) throws DistributionException;

  public ResultSet getStatistics(String distrId, Date startDate, Date endDate) throws DistributionException;

  public State getState(String distrId) throws DistributionException;

  public void resend(String msisdn, String distrId) throws DistributionException;

  public AbstractDynamicMBean getMonitor();

  public void shutdown();

}
 
