package com.eyeline.sponsored.ds.distribution.advert.impl.db;

import com.eyeline.sponsored.ds.AbstractDBTransaction;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.AbstractDBResultSetImpl;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;

import java.sql.*;
import java.util.*;
import java.util.Date;

/**
 *
 * @author artem
 */
public class DBTransaction extends AbstractDBTransaction implements DataSourceTransaction {

  private final DBDistributionDataSource ds;
  private final Properties sql;

  public DBTransaction(DBDistributionDataSource ds, Connection conn, Properties sql) {
    super(conn);
    this.ds = ds;
    this.sql = sql;
  }

  static void checkSql(Properties sql) throws DataSourceException {
    if (!sql.containsKey("delivery.save")) {
      throw new DataSourceException("delivery.save not found");
    }
    if (!sql.containsKey("delivery.update")) {
      throw new DataSourceException("delivery.update not found");
    }
    if (!sql.containsKey("delivery.lookupactive")) {
      throw new DataSourceException("delivery.lookupactive not found");
    }
    if (!sql.containsKey("delivery.lookupactive1")) {
      throw new DataSourceException("delivery.lookupactive1 not found");
    }
    if (!sql.containsKey("delivery.count")) {
      throw new DataSourceException("delivery.count not found");
    }
    if (!sql.containsKey("deliverystat.increase")) {
      throw new DataSourceException("deliverystat.increase not found");
    }
    if (!sql.containsKey("deliverystat.aggregate")) {
      throw new DataSourceException("deliverystat.aggregate not found");
    }
  }

  public void save(DeliveryImpl delivery) throws DataSourceException {
    PreparedStatement ps = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("delivery.save"));
      ps.setString(1, delivery.getSubscriberAddress());
      ps.setString(2, delivery.getDistributionName());
      ps.setInt(3, delivery.getSended());
      ps.setInt(4, delivery.getTotal());
      ps.setTimestamp(5, new Timestamp(delivery.getStartDate().getTime()));
      ps.setTimestamp(6, new Timestamp(delivery.getEndDate().getTime()));
      ps.setTimestamp(7, new Timestamp(delivery.getSendDate().getTime()));
      ps.setString(8, delivery.getTimezone().getID());

      ps.executeUpdate();

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(null, ps);
    }
  }

  public void update(DeliveryImpl delivery) throws DataSourceException {
    PreparedStatement ps = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("delivery.update"));
      ps.setInt(1, delivery.getSended());
      ps.setInt(2, delivery.getTotal());
      ps.setTimestamp(3, new Timestamp(delivery.getStartDate().getTime()));
      ps.setTimestamp(4, new Timestamp(delivery.getEndDate().getTime()));
      ps.setTimestamp(5, new Timestamp(delivery.getSendDate().getTime()));
      ps.setString(6, delivery.getTimezone().getID());
      ps.setString(7, delivery.getSubscriberAddress());
      ps.setString(8, delivery.getDistributionName());

      ps.executeUpdate();

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(null, ps);
    }
  }

  public void remove(DeliveryImpl delivery) throws DataSourceException {

  }

  public List<Delivery> lookupActiveDeliveries(Date end, int limit) throws DataSourceException {
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("delivery.lookupactive"));
      ps.setTimestamp(1, new Timestamp(end.getTime()));
      ps.setInt(2, limit);

      rs = ps.executeQuery();

      final List<Delivery> result = new ArrayList<Delivery>(limit);

      while (rs.next()) {
        final DeliveryImpl delivery = new DeliveryImpl(ds);
        delivery.setSubscriberAddress(rs.getString(1));
        delivery.setDistributionName(rs.getString(2));
        delivery.setSended(rs.getInt(3));
        delivery.setTotal(rs.getInt(4));
        delivery.setStartDate(new Date(rs.getTimestamp(5).getTime()));
        delivery.setEndDate(new Date(rs.getTimestamp(6).getTime()));
        delivery.setSendDate(new Date(rs.getTimestamp(7).getTime()));
        delivery.setTimezone(TimeZone.getTimeZone(rs.getString(8)));
        result.add(delivery);
      }

      return result;

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs,ps);
    }
  }

  public List<Delivery> lookupActiveDeliveries(Date start, Date end) throws DataSourceException {
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("delivery.lookupactive1"));
      ps.setTimestamp(1, new Timestamp(start.getTime()));
      ps.setTimestamp(2, new Timestamp(end.getTime()));

      rs = ps.executeQuery();

      final List<Delivery> result = new LinkedList<Delivery>();

      while (rs.next()) {
        final DeliveryImpl delivery = new DeliveryImpl(ds);
        delivery.setSubscriberAddress(rs.getString(1));
        delivery.setDistributionName(rs.getString(2));
        delivery.setSended(rs.getInt(3));
        delivery.setTotal(rs.getInt(4));
        delivery.setStartDate(new Date(rs.getTimestamp(5).getTime()));
        delivery.setEndDate(new Date(rs.getTimestamp(6).getTime()));
        delivery.setSendDate(new Date(rs.getTimestamp(7).getTime()));
        delivery.setTimezone(TimeZone.getTimeZone(rs.getString(8)));
        result.add(delivery);
      }

      return result;

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs,ps);
    }
  }

  public int getDeliveriesCount(Date date, TimeZone timezone, String distrName) throws DataSourceException {
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      ps = conn.prepareStatement(sql.getProperty("delivery.count"));
      ps.setTimestamp(1, new Timestamp(date.getTime()));
      ps.setString(2, timezone.getID());
      ps.setString(3, distrName);

      rs = ps.executeQuery();
      if (!rs.next())
        throw new DataSourceException("Can't invoke deliveries count");

      return rs.getInt(1);

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs, ps);
    }

  }

  public com.eyeline.sponsored.ds.ResultSet<DeliveryStat> aggregateDeliveryStats(Date startDate, Date endDate) throws DataSourceException {
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      ps = conn.prepareStatement(sql.getProperty("deliverystat.aggregate"), ResultSet.CONCUR_READ_ONLY);
      ps.setFetchSize(Integer.MIN_VALUE);
      ps.setDate(1, new java.sql.Date(startDate.getTime()));
      ps.setDate(2, new java.sql.Date(endDate.getTime()));

      rs = ps.executeQuery();
      return new AbstractDBResultSetImpl<DeliveryStat>(rs, ps) {

        public DeliveryStat get() throws DataSourceException {
          try {
            final DeliveryStatImpl result = new DeliveryStatImpl();
            result.setSubscriberAddress(rs.getString(1));
            result.setDelivered(rs.getInt(2));
            return result;
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

  public void updateDeliveryStat(String subscriberAddress, Date date, int deliveredInc) throws DataSourceException {
    PreparedStatement ps = null, ps1 = null;
    try {
      ps = conn.prepareStatement(sql.getProperty("deliverystat.increase"));
      ps.setInt(1, deliveredInc);
      ps.setString(2, subscriberAddress);
      ps.setDate(3, new java.sql.Date(date.getTime()));

      final int updatedRows = ps.executeUpdate();

      if (updatedRows == 0) { // If deliverystat record does not exists than create it
        ps1 = conn.prepareStatement(sql.getProperty("deliverystat.save"));
        ps1.setString(1, subscriberAddress);
        ps1.setDate(2, new java.sql.Date(date.getTime()));
        ps1.setInt(3, deliveredInc);
        ps1.executeUpdate();
      }
    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(null, ps1);
      close(null, ps);
    }
  }
}
