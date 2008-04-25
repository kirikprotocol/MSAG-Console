package com.eyeline.sponsored.ds.distribution.advert;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;

import java.util.Collection;
import java.util.Date;
import java.util.TimeZone;

/**
 * User: artem
 * Date: 24.03.2008
 */

public interface DeliveriesDataSource {

  public DataSourceTransaction createInsertTransaction(Date startDate, Date endDate, String distrName, int volume, TimeZone tz, int size) throws DataSourceException;

  public Delivery createDelivery();

  public void lookupDeliveries(Date end, int limit, Collection<Delivery> result) throws DataSourceException;

  public void lookupDeliveries(Date start, Date end, Collection<Delivery> result) throws DataSourceException;

  public boolean hasDeliveries(Date date, TimeZone tz, String distrName) throws DataSourceException;

  public void shutdown();
}
