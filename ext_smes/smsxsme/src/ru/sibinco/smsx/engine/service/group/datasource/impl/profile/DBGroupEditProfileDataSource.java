package ru.sibinco.smsx.engine.service.group.datasource.impl.profile;

import ru.sibinco.smsx.network.dbconnection.ConnectionPool;
import ru.sibinco.smsx.utils.DataSourceException;
import ru.sibinco.smsx.utils.DBDataSource;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;
import ru.sibinco.smsx.engine.service.group.datasource.GroupEditProfileDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.GroupEditProfile;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.ResultSet;

/**
 * User: artem
 * Date: 25.07.2008
 */

public class DBGroupEditProfileDataSource extends DBDataSource implements GroupEditProfileDataSource {

  private final ConnectionPool pool;

  public DBGroupEditProfileDataSource() throws DataSourceException {
    super(DBGroupEditProfileDataSource.class.getResourceAsStream("groupedit.properties"), "");

    pool = ConnectionPoolFactory.createConnectionPool("groupedit", Integer.MAX_VALUE, 60000);
    pool.init(1);
  }

  private void createProfile(GroupEditProfile p) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("profile.save"));

      if (p.getSendSmsNotification() == null)
        ps.setInt(1, 1);
      else
        ps.setInt(1, p.getSendSmsNotification() ? 1 : 0);

      if (p.getLockGroupEdit() == null)
        ps.setInt(2, 1);
      else
        ps.setInt(2, p.getLockGroupEdit() ? 1 : 0);

      ps.setString(3, p.getAddress());

      ps.executeUpdate();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  private void updateSendNotifications(GroupEditProfile profile) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("profile.update.send.notifications"));
      ps.setInt(1, profile.getSendSmsNotification() ? 1 : 0);
      ps.setString(2, profile.getAddress());

      if (ps.executeUpdate() == 0)
        createProfile(profile);
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  private void updateLockGroupEdit(GroupEditProfile profile) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("profile.update.lock.group.edit"));
      ps.setInt(1, profile.getLockGroupEdit() ? 1 : 0);
      ps.setString(2, profile.getAddress());

      if (ps.executeUpdate() == 0)
        createProfile(profile);
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public void saveProfile(GroupEditProfile profile) throws DataSourceException {
    if (profile.getSendSmsNotification() != null && profile.getLockGroupEdit() != null)
      createProfile(profile);

    if (profile.getSendSmsNotification() != null)
      updateSendNotifications(profile);

    if (profile.getLockGroupEdit() != null)
      updateLockGroupEdit(profile);

  }

  public GroupEditProfile loadProfile(String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("profile.load"));
      ps.setString(1, address);
      rs = ps.executeQuery();
      if (!rs.next())
        return null;
      final GroupEditProfile p = new GroupEditProfile(address);
      p.setSendSmsNotification(rs.getInt(1) == 1);
      p.setLockGroupEdit(rs.getInt(2) == 1);
      return p;
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
