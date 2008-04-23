package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries;

import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;

import java.util.TimeZone;
import java.util.Date;
import java.util.Collection;

/**
 * User: artem
 * Date: 23.04.2008
 */

public interface DeliveriesFile {

  public void updateDelivery(DeliveryImpl d) throws DeliveriesFileException;

  public DeliveriesFileTransaction createTransaction(String distrName, int volume, TimeZone tz, int size) throws DeliveriesFileException;

  public void readDeliveries(final Date startDate, final Date endDate, Collection<Delivery> result) throws DeliveriesFileException;

  public void readDeliveries(final Date date, final int limit, Collection<Delivery> result) throws DeliveriesFileException;

  public int getDeliveriesCount(final Date date, TimeZone tz, String distrName) throws DeliveriesFileException;

  public void close() throws DeliveriesFileException;
}
