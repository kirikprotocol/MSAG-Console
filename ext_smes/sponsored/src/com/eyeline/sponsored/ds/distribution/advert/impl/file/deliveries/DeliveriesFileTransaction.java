package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;

/**
 * User: artem
 * Date: 23.04.2008
 */

interface DeliveriesFileTransaction extends DataSourceTransaction {
  public void saveDelivery(DeliveryImpl d) throws DataSourceException;  
}
