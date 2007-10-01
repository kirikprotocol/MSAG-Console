package ru.sibinco.smsx.engine.service.sponsored.datasource;

import com.eyeline.sme.utils.ds.DBDataSource;
import com.eyeline.sme.utils.ds.DataSourceException;

import java.sql.*;
import java.util.Collection;
import java.util.LinkedList;

import snaq.db.ConnectionPool;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;

/**
 * User: artem
 * Date: 02.07.2007
 */

public class DBSponsoredDataSource extends DBDataSource implements SponsoredDataSource {

  private final ConnectionPool pool;

  public DBSponsoredDataSource(String sqlFile, String prefix) throws DataSourceException {
    super(sqlFile, prefix);
    pool = ConnectionPoolFactory.createConnectionPool("sponsored", 10, 60000);
    pool.init(1);
  }

  public void saveDistributionInfo(DistributionInfo info) throws DataSourceException {
    Connection conn=null;
    PreparedStatement stmt=null;
    ResultSet rs=null;
    try {

      conn=pool.getConnection();
      if(info.isExists()) {//update
        stmt = conn.prepareStatement(getSql("distribution.info.update"));
        stmt.setTimestamp(1,new Timestamp(System.currentTimeMillis()));
        stmt.setInt(2, info.getTodayCount());
        stmt.setInt(3, info.getCnt());
        stmt.setString(4,info.getAbonent());
        stmt.executeUpdate();
      }else{//insert
        stmt = conn.prepareStatement(getSql("distribution.info.insert"));
        stmt.setString(1,info.getAbonent());
        stmt.setTimestamp(2,new Timestamp(System.currentTimeMillis()));
        stmt.setInt(3, info.getTodayCount());
        stmt.setLong(4, info.getCurrentCount());
        stmt.setInt(5, info.getCnt());
        stmt.executeUpdate();
      }

      info.setExists(true);
    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs, stmt, conn);
    }
  }

  public DistributionInfo getDistributionInfoByAbonent(String abonent) throws DataSourceException {
    Connection conn=null;
    PreparedStatement stmt=null;
    ResultSet rs=null;
    try {
      conn=pool.getConnection();
      stmt = conn.prepareStatement(getSql("distribution.info.load.by.abonent"));
      stmt.setString(1,abonent);
      rs=stmt.executeQuery();
      if (!rs.next())
        return null;

      final DistributionInfo info = new DistributionInfo();
      info.setAbonent(rs.getString(1));
      info.setTodayCount(rs.getInt(2));
      info.setCurrentCount(rs.getInt(3));
      info.setCnt(rs.getInt(4));
      info.setExists(true);
      return info;
    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs, stmt, conn);
    }
  }

  public void decreaseMessagesCount(String abonent) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("decrease.messages.count"));
      ps.setString(1, abonent);
      ps.executeUpdate();

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(null, ps, conn);
    }
  }

  public int getTotalMessagesCount(int subscriptionCount) throws DataSourceException {

    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("get.total.messages.count"));
      ps.setInt(1, subscriptionCount);
      rs = ps.executeQuery();
      if (rs.next())
        return rs.getInt(1);

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs, ps, conn);
    }

    return 0;
  }

  public int getTotalAbonentsCount(int subscriptionCount) throws DataSourceException {

    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("get.total.abonents.count"));
      ps.setInt(1, subscriptionCount);
      rs = ps.executeQuery();
      if (rs.next())
        return rs.getInt(1);

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs, ps, conn);
    }

    return 0;
  }

  public Collection getAbonents(int subscriptionCount, int start, int portion) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    final LinkedList result = new LinkedList();

    // Load abonents
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("get.abonents"));
      ps.setInt(1, subscriptionCount);
      ps.setInt(2, start);
      ps.setInt(3, portion);

      rs = ps.executeQuery();

      while (rs.next())
        result.add(rs.getString(1));

      return result;

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs, ps, conn);
    }
  }

  public void updateSponsoredStatus(String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("delivery.stats.update"));
      ps.setString(1, address);

      int cnt = ps.executeUpdate();
      if (cnt == 0) {
        ps = conn.prepareStatement(getSql("delivery.stats.insert"));
        ps.setString(1, address);
        ps.executeUpdate();
      }

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(null, ps, conn);
    }
  }

  public void release() {
    pool.release();
  }

}
