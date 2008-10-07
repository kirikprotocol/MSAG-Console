package mobi.eyeline.smsquiz.distribution;

import java.util.Date;

public interface DistributionManager {
    
    public String createDistribution(Distribution distr);
	public ResultSet getStatistics(String id, Date startDate, Date endDate);

}
 
