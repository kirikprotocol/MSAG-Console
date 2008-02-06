package com.eyeline.sponsored.ds.subscription.impl.db;

import com.eyeline.sponsored.ds.*;
import com.eyeline.sponsored.ds.subscription.Distribution;
import com.eyeline.sponsored.ds.subscription.SubscriptionRow;
import com.eyeline.sponsored.ds.subscription.VolumeStat;
import com.eyeline.sponsored.ds.subscription.impl.AbstractSubscriptionRowImpl;
import com.eyeline.sponsored.ds.subscription.impl.AbstractVolumeStatImpl;

import java.sql.*;
import java.sql.ResultSet;
import java.util.Date;
import java.util.Properties;
import java.util.TimeZone;

/**
 *
 * @author artem
 */
class DBTransaction extends AbstractDBTransaction implements DataSourceTransaction {

  private final DBSubscriptionDataSource ds;
  private final Properties sql;

  DBTransaction(DBSubscriptionDataSource ds, Connection conn, Properties sql) throws SQLException {
    super(conn);
    this.ds = ds;    
    this.sql = sql;    
  }

  static void checkSql(Properties sql) throws DataSourceException {
    if (!sql.containsKey("subscriber.save")) {
      throw new DataSourceException("subscriber.save not found");
    }
    if (!sql.containsKey("subscriber.remove")) {
      throw new DataSourceException("subscriber.remove not found");
    }
    if (!sql.containsKey("subscriber.lookup")) {
      throw new DataSourceException("subscriber.lookup not found");
    }
    if (!sql.containsKey("distribution.save")) {
      throw new DataSourceException("distribution.save not found");
    }
    if (!sql.containsKey("distribution.remove")) {
      throw new DataSourceException("distribution.remove not found");
    }
    if (!sql.containsKey("distribution.lookup")) {
      throw new DataSourceException("distribution.lookup not found");
    }
    if (!sql.containsKey("subscription.save")) {
      throw new DataSourceException("subscription.save not found");
    }
    if (!sql.containsKey("subscription.update")) {
      throw new DataSourceException("subscription.update not found");
    }
    if (!sql.containsKey("subscription.remove")) {
      throw new DataSourceException("subscription.remove not found");
    }
    if (!sql.containsKey("subscription.lookupactive")) {
      throw new DataSourceException("subscription.lookupactive not found");
    }
    if (!sql.containsKey("volumestat.get")) {
      throw new DataSourceException("volumestat.get not found");
    }
  }

  public void save(SubscriberImpl subscriber) throws DataSourceException {
    PreparedStatement ps = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("subscriber.save"));
      ps.setString(1, subscriber.getAddress());
      ps.setString(2, subscriber.getTimeZone().getID());
      ps.executeUpdate();
    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(null, ps);
    }
  }

  public void remove(SubscriberImpl subscriber) throws DataSourceException {
    PreparedStatement ps = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("subscriber.remove"));
      ps.setString(1, subscriber.getAddress());
      ps.executeUpdate();
    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(null, ps);
    }
  }

  private Distribution.Status getDistributionStatus(int status) {
    switch (status) {
      case 0:
        return Distribution.Status.OPENED;
      default:
        return Distribution.Status.CLOSED;
    }
  }

  public void save(DistributionImpl distribution) throws DataSourceException {
    PreparedStatement ps = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("distribution.save"));
      ps.setString(1, distribution.getName());
      switch (distribution.getStatus()) {
        case OPENED:
          ps.setInt(2, 0);
          break;
        default:
          ps.setInt(2, 1);
      }
      ps.executeUpdate();
    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(null, ps);
    }
  }

  public void remove(DistributionImpl distribution) throws DataSourceException {
    PreparedStatement ps = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("distribution.remove"));
      ps.setString(1, distribution.getName());
      ps.executeUpdate();
    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(null, ps);
    }
  }

  public void save(SubscriptionImpl subscription) throws DataSourceException {
    PreparedStatement ps = null;
    try {
      ps = conn.prepareStatement(sql.getProperty(subscription.isExists() ? "subscription.update" : "subscription.save"));
      ps.setString(1, subscription.getSubscriberAddress());
      ps.setString(2, subscription.getDistributionName());
      ps.setInt(3, subscription.getVolume());
      ps.setTimestamp(4, new Timestamp(subscription.getStartDate().getTime()));
      if (subscription.getEndDate() == null) {
        ps.setNull(5, Types.TIMESTAMP);
      } else {
        ps.setTimestamp(5, new Timestamp(subscription.getEndDate().getTime()));
      }

      if (subscription.isExists()) {
        ps.setInt(6, subscription.getId());
      }

      ps.executeUpdate();
    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(null, ps);
    }
  }

  public void remove(SubscriptionImpl subscription) throws DataSourceException {
    if (!subscription.isExists()) {
      throw new DataSourceException("Subscription does not loaded");
    }

    PreparedStatement ps = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("subscription.remove"));
      ps.setInt(1, subscription.getId());
      ps.executeUpdate();
    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(null, ps);
    }
  }

  public SubscriberImpl lookupSubscriber(String subscriberAddress) throws DataSourceException {
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      ps = conn.prepareStatement(sql.getProperty("subscriber.lookup"));
      ps.setString(1, subscriberAddress);

      rs = ps.executeQuery();
      if (rs.next()) {
        final SubscriberImpl result = new SubscriberImpl(ds);
        result.setAddress(rs.getString(1));
        result.setTimeZone(TimeZone.getTimeZone(rs.getString(2)));
        return result;
      }
      return null;

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs, ps);
    }
  }

  public DistributionImpl lookupDistribution(String distributionName) throws DataSourceException {
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("distribution.lookup"));
      ps.setString(1, distributionName);

      rs = ps.executeQuery();
      if (rs.next()) {
        final DistributionImpl result = new DistributionImpl(ds);
        result.setName(rs.getString(1));
        result.setStatus(getDistributionStatus(rs.getInt(2)));
        return result;
      }
      return null;

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs, ps);
    }
  }

  public SubscriptionImpl lookupActiveSubscription(String subscriberAddress, String distributionName, Date date) throws DataSourceException {
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("subscription.lookupactive"));
      ps.setString(1, subscriberAddress);
      ps.setString(2, distributionName);
      ps.setTimestamp(3, new Timestamp(date.getTime()));

      rs = ps.executeQuery();
      if (rs.next()) {
        final SubscriptionImpl result = new SubscriptionImpl(ds);
        result.setId(rs.getInt(1));
        result.setSubscriberAddress(rs.getString(2));
        result.setDistributionName(rs.getString(3));
        result.setVolume(rs.getInt(4));
        result.setStartDate(new Date(rs.getTimestamp(5).getTime()));
        result.setEndDate(rs.getTimestamp(6) == null ? null : new Date(rs.getTimestamp(6).getTime()));
        return result;
      }
      return null;

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs, ps);
    }
  }  

  public com.eyeline.sponsored.ds.ResultSet<SubscriptionRow> lookupFullSubscriptionInfo(String distributionName, Date date, TimeZone timezone) throws DataSourceException {
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("subscription.lookupfull"), ResultSet.CONCUR_READ_ONLY);
      ps.setFetchSize(Integer.MIN_VALUE);
      ps.setString(1, distributionName);
      ps.setTimestamp(2, new Timestamp(date.getTime()));
      ps.setTimestamp(3, new Timestamp(date.getTime()));
      ps.setString(4, timezone.getID());

      rs = ps.executeQuery();

      return new AbstractDBResultSetImpl(rs, ps) {

        public SubscriptionRow get() throws DataSourceException {
          final SubscriptionImpl subscription = new SubscriptionImpl(ds);
          try {
            subscription.setId(rs.getInt(1));
            subscription.setSubscriberAddress(rs.getString(2));
            subscription.setDistributionName(rs.getString(3));
            subscription.setVolume(rs.getInt(4));
            subscription.setStartDate(new Date(rs.getTimestamp(5).getTime()));
            subscription.setEndDate(rs.getTimestamp(6) == null ? null : new Date(rs.getTimestamp(6).getTime()));

            final DistributionImpl distribution = new DistributionImpl(ds);
            distribution.setName(rs.getString(7));
            distribution.setStatus(getDistributionStatus(rs.getInt(8)));

            final SubscriberImpl subscriber = new SubscriberImpl(ds);
            subscriber.setAddress(rs.getString(9));
            subscriber.setTimeZone(TimeZone.getTimeZone(rs.getString(10)));

            final AbstractSubscriptionRowImpl row = new AbstractSubscriptionRowImpl();
            row.setSubscription(subscription);
            row.setSubscriber(subscriber);
            row.setDistribution(distribution);

            return row;
          } catch (SQLException e) {
            throw new DataSourceException(e);
          }
        }
      };


    } catch (SQLException e) {
      close(rs, ps);
      throw new DataSourceException(e);
    }
  }

  public com.eyeline.sponsored.ds.ResultSet<VolumeStat> getVolumeStat(String distributionName, Date date, TimeZone timezone) throws DataSourceException {
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      ps = conn.prepareStatement(sql.getProperty("volumestat.get"), ResultSet.CONCUR_READ_ONLY);
      ps.setFetchSize(Integer.MIN_VALUE);
      ps.setString(1, distributionName);
      ps.setString(2, timezone.getID());
      ps.setTimestamp(3, new Timestamp(date.getTime()));
      ps.setTimestamp(4, new Timestamp(date.getTime()));

      rs = ps.executeQuery();

      return new AbstractDBResultSetImpl<VolumeStat>(rs,ps) {
        public VolumeStat get() throws DataSourceException {
          try {
            final AbstractVolumeStatImpl result = new AbstractVolumeStatImpl();
            result.setVolume(rs.getInt(1));
            result.setNumberOfSubscribers(rs.getInt(2));
            return result;
          } catch (SQLException e) {
            throw new DataSourceException(e);
          }
        }
      };
      
    } catch (SQLException e) {
      close(rs,ps);
      throw new DataSourceException(e);
    }
  }
}
