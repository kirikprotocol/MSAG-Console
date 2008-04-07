package com.eyeline.sponsored.ds.distribution.advert;

import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.DataSourceException;

import java.util.List;
import java.util.Date;
import java.util.TimeZone;

/**
 * User: artem
 * Date: 24.03.2008
 */

public interface DeliveriesDataSource {
  public DataSourceTransaction createTransaction() throws DataSourceException;

  public Delivery createDelivery();

  public List<Delivery> lookupActiveDeliveries(Date end, int limit) throws DataSourceException;

  public List<Delivery> lookupActiveDeliveries(Date start, Date end) throws DataSourceException;

  public int getDeliveriesCount(Date date, TimeZone tz, String distrName) throws DataSourceException;

  public void shutdown();
}
