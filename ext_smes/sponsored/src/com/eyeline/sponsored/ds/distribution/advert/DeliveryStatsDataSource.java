package com.eyeline.sponsored.ds.distribution.advert;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.ResultSet;

import java.util.Date;

/**
 * User: artem
 * Date: 24.03.2008
 */

public interface DeliveryStatsDataSource {

  public void updateDeliveryStat(String subscriberAddress, Date startDate, int deliveredInc) throws DataSourceException;

  public ResultSet<DeliveryStat> aggregateDeliveryStats(Date startDate, Date endDate) throws DataSourceException;

  public ResultSet<DeliveryStat> getDeliveryStats(Date date) throws DataSourceException;

  public void shutdown();
}
