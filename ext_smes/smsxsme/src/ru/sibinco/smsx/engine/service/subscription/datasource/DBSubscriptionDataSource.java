package ru.sibinco.smsx.engine.service.subscription.datasource;

import ru.sibinco.smsx.network.dbconnection.ConnectionPool;
import ru.sibinco.smsx.utils.DataSourceException;
import ru.sibinco.smsx.utils.DBDataSource;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;


/**
 * User: artem
 * Date: 28.07.2008
 */

public class DBSubscriptionDataSource extends DBDataSource implements SubscriptionDataSource  {

  private final ConnectionPool pool;

  public DBSubscriptionDataSource() throws DataSourceException {
    super(DBSubscriptionDataSource.class.getResourceAsStream("subscription.properties"), "");

    pool = ConnectionPoolFactory.createConnectionPool("subscription", Integer.MAX_VALUE, 60000);
    pool.init(1);
  }

  public boolean checkSubscription(String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("subscription.lookup"));
      ps.setString(1, address);

      rs = ps.executeQuery();
      return rs.next();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  public void shutdown() {
    pool.release();
  }
}
