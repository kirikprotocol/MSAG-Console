package mobi.eyeline.smsquiz.distribution;

import mobi.eyeline.smsquiz.storage.ResultSet;

import java.util.Date;

public interface DistributionManager {

  public String createDistribution(Distribution distr) throws DistributionException;

  public ResultSet getStatistics(String id, Date startDate, Date endDate) throws DistributionException;

}
 
