package com.eyeline.sponsored.ds.distribution.advert;

import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.DataSourceException;

import java.util.List;
import java.util.Date;
import java.util.TimeZone;
import java.util.Collection;

/**
 * User: artem
 * Date: 24.03.2008
 */

public interface DeliveriesDataSource {
  public DataSourceTransaction createTransaction() throws DataSourceException;

  public Delivery createDelivery();

  public void lookupDeliveries(Date end, int limit, Collection<Delivery> result) throws DataSourceException;

  public void lookupDeliveries(Date start, Date end, Collection<Delivery> result) throws DataSourceException;

  public int getDeliveriesCount(Date date, TimeZone tz, String distrName) throws DataSourceException;

  public void shutdown();
}
