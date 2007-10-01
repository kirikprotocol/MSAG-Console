package ru.sibinco.smsx.engine.service.nick.datasource;

import com.eyeline.sme.utils.ds.DBDataSource;
import com.eyeline.sme.utils.ds.DataSourceException;
import snaq.db.ConnectionPool;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;

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

  public DBNickDataSource(String s, String s1) throws DataSourceException {
    super(s, s1);

    pool = ConnectionPoolFactory.createConnectionPool("Nick", 5, 60000);
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
      throw new DataSourceException(e);
    } finally {
      close(null, ps, conn);
    }
  }

  public void deleteNick(String abonentAddress) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("nick.delete"));

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(null, ps, conn);
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
      throw new DataSourceException(e);
    } finally {
      close(rs, ps, conn);
    }
  }
}
