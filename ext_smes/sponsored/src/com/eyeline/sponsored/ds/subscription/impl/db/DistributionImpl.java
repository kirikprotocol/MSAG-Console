package com.eyeline.sponsored.ds.subscription.impl.db;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.subscription.impl.AbstractDistributionImpl;


/**
 *
 * @author artem
 */
public class DistributionImpl extends AbstractDistributionImpl {

  private final DBSubscriptionDataSource ds;

  public DistributionImpl(DBSubscriptionDataSource ds) {
    this.ds = ds;
  }

  public void save() throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = ds.createDBTransaction(true);
      tx.save(this);
    } finally {
      if (tx != null) {
        tx.close();
      }
    }
  }

  public void save(DataSourceTransaction tx) throws DataSourceException {
    ((DBTransaction) tx).save(this);
  }

  public void remove() throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = ds.createDBTransaction(true);
      tx.remove(this);      
    } finally {
      if (tx != null) {
        tx.close();
      }
    }
  }

  public void remove(DataSourceTransaction tx) throws DataSourceException {
    ((DBTransaction) tx).remove(this);
  }
}
