package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries;

import com.eyeline.sponsored.ds.distribution.advert.impl.AbstractDeliveryImpl;
import com.eyeline.sponsored.ds.distribution.advert.Volume;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;

/**
 * User: artem
 * Date: 28.03.2008
 */

public class DeliveryImpl extends AbstractDeliveryImpl {

  private long id = -1;
  private final FileDeliveriesDataSource ds;

  public DeliveryImpl(FileDeliveriesDataSource ds) {
    this.ds = ds;
  }

  public long getId() {
    return id;
  }

  public void setId(long id) {
    this.id = id;
  }

  public void update() throws DataSourceException {
    ds.updateDelivery(this);
  }

  public void save() throws DataSourceException {
    ds.updateDelivery(this);
  }

  public void save(DataSourceTransaction tx) throws DataSourceException {
    ds.saveDelivery(this, tx);
  }

  public void remove() throws DataSourceException {
    throw new DataSourceException("Operation 'remove' is not supported");
  }

  public void remove(DataSourceTransaction tx) throws DataSourceException {
    throw new DataSourceException("Operation 'remove' is not supported");
  }
}
