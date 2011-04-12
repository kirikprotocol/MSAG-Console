package ru.sibinco.smsx.engine.service.group.datasource.impl.principal;

import ru.sibinco.smsx.engine.service.group.datasource.Principal;
import ru.sibinco.smsx.engine.service.group.datasource.PrincipalDataSource;
import ru.sibinco.smsx.network.dbconnection.ConnectionPool;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;
import ru.sibinco.smsx.utils.DBDataSource;
import ru.sibinco.smsx.utils.DataSourceException;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;

/**
 * User: artem
 * Date: 02.12.2009
 */
public class DBPrincipalDataSource extends DBDataSource implements PrincipalDataSource {

  private final ConnectionPool pool;

  public DBPrincipalDataSource() throws DataSourceException {
    super(DBPrincipalDataSource.class.getResourceAsStream("principals.properties"), "");

    pool = ConnectionPoolFactory.createConnectionPool("groupeditprincipals", Integer.MAX_VALUE, 60000);
    pool.init(1);
  }

  public void addPrincipal(Principal principal) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("add"));
      ps.setString(1, principal.getAddress());
      ps.setInt(2, principal.getMaxLists());
      ps.setInt(3, principal.getMaxMembersPerList());

      ps.executeUpdate();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public boolean removePrincipal(String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("remove"));
      ps.setString(1, address);

      return ps.executeUpdate() > 0;
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public Principal getPrincipal(String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("get"));
      ps.setString(1, address);

      rs = ps.executeQuery();

      if (rs.next()) {
        Principal p = new Principal(address);
        p.setMaxLists(rs.getInt(1));
        p.setMaxMembersPerList(rs.getInt(2));
        return p;
      } else
        return null;

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  public Collection<Principal> getPrincipals() throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("all"));

      rs = ps.executeQuery();

      Collection<Principal> result = new ArrayList<Principal>(1000);

      while (rs.next()) {
        Principal p = new Principal(rs.getString(1));
        p.setMaxLists(rs.getInt(2));
        p.setMaxMembersPerList(rs.getInt(3));
        result.add(p);
      }

      return result;

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  public boolean containsPrincipal(String address) throws DataSourceException {
    return getPrincipal(address) != null;
  }

  public void close() {
    pool.release();
  }
}
