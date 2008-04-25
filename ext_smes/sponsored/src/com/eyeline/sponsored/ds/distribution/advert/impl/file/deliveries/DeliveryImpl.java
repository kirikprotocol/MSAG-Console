package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.distribution.advert.impl.AbstractDeliveryImpl;

/**
 * User: artem
 * Date: 28.03.2008
 */

class DeliveryImpl extends AbstractDeliveryImpl {

  public void update() throws DataSourceException {
    throw new DataSourceException("Can't update delivery from file data source");
  }

  public void save() throws DataSourceException {
    update();
  }

  public void save(DataSourceTransaction tx) throws DataSourceException {
    ((DeliveriesFileTransaction)tx).saveDelivery(this);
  }

  public void remove() throws DataSourceException {
    throw new DataSourceException("Operation 'remove' is not supported");
  }

  public void remove(DataSourceTransaction tx) throws DataSourceException {
    throw new DataSourceException("Operation 'remove' is not supported");
  }
}
