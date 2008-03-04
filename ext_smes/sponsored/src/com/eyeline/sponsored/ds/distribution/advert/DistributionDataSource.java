package com.eyeline.sponsored.ds.distribution.advert;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.ResultSet;

import java.util.Date;
import java.util.TimeZone;
import java.util.List;

/**
 *
 * @author artem
 */
public interface DistributionDataSource {

  public DataSourceTransaction createTransaction() throws DataSourceException;
  
  public Delivery createDelivery();

  /**
   * Fast update delivery statistic record or create it if not exists
   * @param subscriberAddress subscriber address
   * @param startDate start date of statistic record
   * @param deliveredInc delivered counter increase
   * @throws com.eyeline.sponsored.ds.DataSourceException
   */
  public void updateDeliveryStat(String subscriberAddress, Date startDate, int deliveredInc) throws DataSourceException;

  public void updateDeliveryStat(String subscriberAddress, Date startDate, int deliveredInc, DataSourceTransaction tx) throws DataSourceException;

  public ResultSet<DeliveryStat> aggregateDeliveryStats(Date startDate, Date endDate, DataSourceTransaction tx) throws DataSourceException;

  public List<Delivery> lookupActiveDeliveries(Date end, int limit) throws DataSourceException;

  public List<Delivery> lookupActiveDeliveries(Date start, Date end) throws DataSourceException;

  public int getDeliveriesCount(Date date, TimeZone tz, String distrName) throws DataSourceException;

  public int getDeliveriesCount(Date date, TimeZone tz, String distrName, DataSourceTransaction tx) throws DataSourceException;
}
