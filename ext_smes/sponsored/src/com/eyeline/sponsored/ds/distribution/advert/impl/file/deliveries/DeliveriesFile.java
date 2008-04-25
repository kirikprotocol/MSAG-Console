package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;

import java.util.Collection;
import java.util.Date;
import java.util.TimeZone;

/**
 * User: artem
 * Date: 23.04.2008
 */

interface DeliveriesFile {

  public DeliveriesFileTransaction createInsertTransaction(Date startDate, Date endDate, String distrName, int volume, TimeZone tz, int size) throws DataSourceException;

  public void lookupDeliveries(final Date startDate, final Date endDate, Collection<Delivery> result) throws DataSourceException;
  public void lookupDeliveries(final Date date, final int limit, Collection<Delivery> result) throws DataSourceException;
  public boolean hasDeliveries(Date endDate, TimeZone tz, String distrName) throws DataSourceException;

  public void close() throws DataSourceException;
}
