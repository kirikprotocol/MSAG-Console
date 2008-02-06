package com.eyeline.sponsored.ds.distribution.advert.impl.db;

import com.eyeline.sponsored.ds.distribution.advert.impl.AbstractDeliveryImpl;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;

/**
 *
 * @author artem
 */
class DeliveryImpl extends AbstractDeliveryImpl {

  private final DBDistributionDataSource ds;

  public DeliveryImpl(DBDistributionDataSource ds) {
    this.ds = ds;
  }

  public void save() throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = ds.createDBTransaction(true);
      tx.save(this);
    } finally {
      if (tx != null)
        tx.close();
    }
  }

  public void save(DataSourceTransaction tx) throws DataSourceException {
    ((DBTransaction)tx).save(this);
  }

  public void remove() throws DataSourceException {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  public void remove(DataSourceTransaction tx) throws DataSourceException {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  public void update() throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = ds.createDBTransaction(true);
      tx.update(this);      
    } finally {
      if (tx != null)
        tx.close();
    }
  }
}
