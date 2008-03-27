package com.eyeline.sponsored.ds.distribution.advert.impl.db;

import com.eyeline.sponsored.ds.AbstractDBDataSource;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.ResultSet;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;
import com.eyeline.sponsored.ds.distribution.advert.DeliveriesDataSource;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStatsDataSource;

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
public class DBDistributionDataSource extends AbstractDBDataSource implements DeliveriesDataSource, DeliveryStatsDataSource {

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

  public ResultSet<DeliveryStat> aggregateDeliveryStats(Date startDate, Date endDate) throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = createDBTransaction(true);
      return tx.aggregateDeliveryStats(startDate, endDate);
    } finally {
      if (tx != null) {
        tx.close();
      }
    }
  }

  public ResultSet<DeliveryStat> getDeliveryStats(Date date) throws DataSourceException {
    return null;
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

  public List<Delivery> lookupActiveDeliveries(Date start, Date end) throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = createDBTransaction(true);
      return tx.lookupActiveDeliveries(start, end);
    } finally {
      if (tx != null) {
        tx.close();
      }
    }
  }

  public int getDeliveriesCount(Date date, TimeZone tz, String distrName) throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = createDBTransaction(true);
      return tx.getDeliveriesCount(date, tz, distrName);
    } finally {
      if (tx != null) {
        tx.close();
      }
    }
  }

  public int getDeliveriesCount(Date date, TimeZone tz, String distrName, DataSourceTransaction tx) throws DataSourceException {
    return ((DBTransaction)tx).getDeliveriesCount(date, tz, distrName);
  }
}
