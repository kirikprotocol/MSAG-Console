package com.eyeline.sponsored.ds.distribution.advert;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.ResultSet;

import java.util.Date;

/**
 * User: artem
 * Date: 24.03.2008
 */

public interface DeliveryStatsDataSource {

  public void addDeliveryStat(String subscriberAddress, int advertiserId, Date startDate, int deliveredInc, int sendedInc) throws DataSourceException;

  public ResultSet<DeliveryStat> aggregateDeliveryStats(Date startDate, Date endDate) throws DataSourceException;

  public ResultSet<DeliveryStat> getDeliveryStats(Date date) throws DataSourceException;

  public void shutdown();
}
