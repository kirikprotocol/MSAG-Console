package com.eyeline.sponsored.ds.subscription.impl.db;

import com.eyeline.sponsored.ds.AbstractDBDataSource;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.ResultSet;
import com.eyeline.sponsored.ds.subscription.*;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.Date;
import java.util.Properties;
import java.util.TimeZone;

/**
 *
 * @author artem
 */
public class DBSubscriptionDataSource extends AbstractDBDataSource implements SubscriptionDataSource {

  private final Properties sql;

  public DBSubscriptionDataSource(Properties sql) throws DataSourceException {
    // Check SQL
    DBTransaction.checkSql(sql);
    this.sql = sql;
  }  

  public DataSourceTransaction createTransaction() throws DataSourceException {
    return createDBTransaction(false);
  }

  DBTransaction createDBTransaction(boolean autoCommit) throws DataSourceException {
    try {
      final Connection conn = getConnection();
      if (autoCommit != conn.getAutoCommit())
        conn.setAutoCommit(autoCommit);
      return new DBTransaction(this, conn, sql);
    } catch (SQLException ex) {
      throw new DataSourceException(ex);
    }
  }

  public Subscriber createSubscriber() {
    return new SubscriberImpl(this);
  }

  public Distribution createDistribution() {
    return new DistributionImpl(this);
  }

  public Subscription createSubscription() {
    return new SubscriptionImpl(this);
  }

  public Subscriber lookupSubscriber(String subscriberAddress) throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = createDBTransaction(true);
      return tx.lookupSubscriber(subscriberAddress);
    } finally {
      if (tx != null) {
        tx.close();
      }
    }
  }

  public Subscriber lookupSubscriber(String subscriberAddress, DataSourceTransaction tx) throws DataSourceException {
    return ((DBTransaction) tx).lookupSubscriber(subscriberAddress);
  }

  public Distribution lookupDistribution(String distributionName) throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = createDBTransaction(true);
      return tx.lookupDistribution(distributionName);
    } finally {
      if (tx != null) {
        tx.close();
      }
    }
  }

  public Distribution lookupDistribution(String distributionName, DataSourceTransaction tx) throws DataSourceException {
    return ((DBTransaction) tx).lookupDistribution(distributionName);
  }

  public Subscription lookupActiveSubscription(String subscriberAddress, String distributionName, Date date) throws DataSourceException {
    DBTransaction tx = null;
    try {
      tx = createDBTransaction(true);
      return tx.lookupActiveSubscription(subscriberAddress, distributionName, date);
    } finally {
      if (tx != null) {
        tx.close();
      }
    }
  }

  public Subscription lookupActiveSubscription(String subscriberAddress, String distributionName, Date date, DataSourceTransaction tx) throws DataSourceException {
    return ((DBTransaction) tx).lookupActiveSubscription(subscriberAddress, distributionName, date);
  }

  public ResultSet<SubscriptionRow> lookupFullSubscriptionInfo(String distributionName, Date date, TimeZone timezone, DataSourceTransaction tx) throws DataSourceException {
    return ((DBTransaction)tx).lookupFullSubscriptionInfo(distributionName, date, timezone);
  }

  public ResultSet<VolumeStat> getVolumeStats(String distributionName, Date date, TimeZone timezone, DataSourceTransaction tx) throws DataSourceException {
    return ((DBTransaction)tx).getVolumeStat(distributionName, date, timezone);
  }
}
