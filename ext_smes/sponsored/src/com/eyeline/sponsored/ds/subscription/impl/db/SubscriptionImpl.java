package com.eyeline.sponsored.ds.subscription.impl.db;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.subscription.impl.AbstractSubscriptionImpl;

/**
 *
 * @author artem
 */
public class SubscriptionImpl extends AbstractSubscriptionImpl {

  private final DBSubscriptionDataSource ds;
  private int id = -1;

  public SubscriptionImpl(DBSubscriptionDataSource ds) {
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
  
  public int getId() {
    return id;
  }

  public void setId(int id) {
    this.id = id;
  }
  
  public boolean isExists() {
    return id >=0;
  }
}
