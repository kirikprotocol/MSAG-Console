package mobi.eyeline.smsquiz.distribution;

import mobi.eyeline.smsquiz.storage.ResultSet;

import java.util.Date;

public interface DistributionManager {

  public String createDistribution(Distribution distr, Runnable task, String errorFile) throws DistributionException;

  public ResultSet getStatistics(String id, Date startDate, Date endDate) throws DistributionException;

  public String repairStatus(String id, String errorFile, Runnable task, Distribution distribution) throws DistributionException;

  public void shutdown();

}
 
