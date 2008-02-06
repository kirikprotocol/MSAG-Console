package com.eyeline.sponsored.ds.distribution.advert.impl.db;

import com.eyeline.sponsored.ds.AbstractDBDataSource;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.ResultSet;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;
import com.eyeline.sponsored.ds.distribution.advert.DistributionDataSource;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.Date;
import java.util.List;
import java.util.Properties;
import java.util.TimeZone;

/**
 *
 * @author artem
 */
public class DBDistributionDataSource extends AbstractDBDataSource implements DistributionDataSource {

  private final Properties sql;

  public DBDistributionDataSource(Properties sql) throws DataSourceException {
    // Check SQL
    DBTransaction.checkSql(sql);
    this.sql = sql;
  }  

  DBTransaction createDBTransaction(boolean autoCommit) throws DataSourceException {
    try {
      final Connection conn = getConnection();
      if (conn.getAutoCommit() != autoCommit)
        conn.setAutoCommit(autoCommit);
      return new DBTransaction(this, conn, sql);
    } catch (SQLException ex) {
      throw new DataSourceException(ex);
    }
  }

  public DataSourceTransaction createTransaction() throws DataSourceException {
    return createDBTransaction(false);
  }

  public Delivery createDelivery() {
    return new DeliveryImpl(this);
  }

  public void updateDeliveryStat(String subscriberAddress, Date date, int deliveredInc) throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = createDBTransaction(true);
      tx.updateDeliveryStat(subscriberAddress, date, deliveredInc);
    } finally {
      if (tx != null) {
        tx.close();
      }
    }
  }

  public void updateDeliveryStat(String subscriberAddress, Date date, int deliveredInc, DataSourceTransaction tx) throws DataSourceException {
    ((DBTransaction)tx).updateDeliveryStat(subscriberAddress, date, deliveredInc);
  }

  public ResultSet<DeliveryStat> aggregateDeliveryStats(Date startDate, Date endDate, DataSourceTransaction tx) throws DataSourceException {
    return ((DBTransaction)tx).aggregateDeliveryStats(startDate, endDate);
  }

  public List<Delivery> lookupActiveDeliveries(Date end, int limit) throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = createDBTransaction(true);
      return tx.lookupActiveDeliveries(end, limit);
    } finally {
      if (tx != null) {
        tx.close();
      }
    }
  }

  public int getDeliveriesCount(Date date, TimeZone tz) throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = createDBTransaction(true);
      return tx.getDeliveriesCount(date, tz);
    } finally {
      if (tx != null) {
        tx.close();
      }
    }
  }

  public int getDeliveriesCount(Date date, TimeZone tz, DataSourceTransaction tx) throws DataSourceException {
    return ((DBTransaction)tx).getDeliveriesCount(date, tz);
  }
}
