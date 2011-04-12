package ru.sibinco.smsx.engine.service.nick.datasource;

import ru.sibinco.smsx.network.dbconnection.ConnectionPool;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;
import ru.sibinco.smsx.utils.DBDataSource;
import ru.sibinco.smsx.utils.DataSourceException;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.ResultSet;

/**
 * User: artem
 * Date: Sep 6, 2007
 */

public class DBNickDataSource extends DBDataSource implements NickDataSource{

  private final ConnectionPool pool;

  public DBNickDataSource() throws DataSourceException {
    super(DBNickDataSource.class.getResourceAsStream("nick.properties"), "");

    pool = ConnectionPoolFactory.createConnectionPool("Nick", Integer.MAX_VALUE, 60000);
    pool.init(1);
  }

  public void setNick(String abonentAddress, String nick) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("nick.update"));
      ps.setString(1, nick);
      ps.setString(2, abonentAddress);

      int cnt = ps.executeUpdate();
      if (cnt == 0) {
        ps = conn.prepareStatement(getSql("nick.insert"));
        ps.setString(1, nick);
        ps.setString(2, abonentAddress);
        ps.executeUpdate();
      }

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public void deleteNick(String abonentAddress) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("nick.delete"));

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public NickNick getNickByAddress(String abonentAddress) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("nick.load.by.address"));
      ps.setString(1, abonentAddress);

      rs = ps.executeQuery();
      if (rs == null || !rs.next())
        return null;

      return new NickNick(rs.getString(1), rs.getString(2));

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }
}
