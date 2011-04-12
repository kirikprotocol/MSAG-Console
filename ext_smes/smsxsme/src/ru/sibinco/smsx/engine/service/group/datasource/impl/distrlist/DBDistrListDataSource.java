package ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist;

import ru.sibinco.smsx.engine.service.group.datasource.DistrListDataSource;
import ru.sibinco.smsx.engine.service.group.datasource.DistrList;
import ru.sibinco.smsx.network.dbconnection.ConnectionPool;
import ru.sibinco.smsx.utils.DataSourceException;
import ru.sibinco.smsx.utils.DBDataSource;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;

import java.util.Collection;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;


/**
 * User: artem
 * Date: 02.12.2009
 */
public class DBDistrListDataSource extends DBDataSource implements DistrListDataSource {

  private final ConnectionPool pool;
  private AtomicInteger id;
  private Lock idLock = new ReentrantLock();
  private final WeakDistrListCache cache;

  public DBDistrListDataSource() throws DataSourceException {
    super(DBDistrListDataSource.class.getResourceAsStream("distrlist.properties"), "");

    pool = ConnectionPoolFactory.createConnectionPool("groupeditlists", Integer.MAX_VALUE, 60000);
    pool.init(1);

    cache = new WeakDistrListCache();
  }

  private int getNextId() throws DataSourceException {
    if (id == null) {
      try {
        idLock.lock();
        if (id == null)
          id = new AtomicInteger(loadId());
      } finally {
        idLock.unlock();
      }
    }
    return id.incrementAndGet();
  }

  private int loadId() throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("lists.get.max.id"));

      rs = ps.executeQuery();

      return (rs.next()) ? rs.getInt(1) : 0;

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }
  
  public DistrList createDistrList(String name, String owner, int maxElements) throws DataSourceException {
    int listId =  getNextId();

    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("lists.add"));
      ps.setInt(1, listId);
      ps.setString(2, name);
      ps.setString(3, owner == null ? "system" : owner);
      ps.setInt(4, maxElements);
      ps.execute();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }

    DistrList list = new DBDistrList(listId, name, owner, maxElements, this);
    list = cache.add(list);
    return list;
  }

  public void removeDistrList(String name, String owner) throws DataSourceException {
    DistrList list = getDistrList(name, owner);

    Connection conn = null;
    PreparedStatement ps = null;
    PreparedStatement ps1 = null;
    PreparedStatement ps2 = null;
    try {
      conn = pool.getConnection();
      conn.setAutoCommit(false);
      ps = conn.prepareStatement(getSql("lists.remove.by.name"));
      ps.setString(1, name);
      ps.setString(2, owner == null ? "system" : owner);
      ps.execute();

      ps1 = conn.prepareStatement(getSql("members.remove.by.group"));
      ps1.setInt(1, list.getId());
      ps1.execute();

      ps2 = conn.prepareStatement(getSql("submitters.remove.by.group"));
      ps2.setInt(1, list.getId());
      ps2.execute();

      conn.commit();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps1, null);
      _close(null, ps2, null);
      try {
        if (conn != null)
          conn.setAutoCommit(true);
      } catch (SQLException e) {
      }
      _close(null, ps, conn);
    }

    cache.remove(list);
  }

  public void removeDistrLists(String owner) throws DataSourceException {
    Collection<DistrList> lists = getDistrLists(owner);

    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("lists.remove.by.owner"));
      ps.setString(1, owner == null ? "system" : owner);
      ps.execute();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }

    cache.removeAll(lists);
  }

  public DistrList getDistrList(String name, String owner) throws DataSourceException {

    DistrList list = cache.get(owner, name);
    if (list != null)
      return list;

    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("lists.get.by.name"));
      ps.setString(1, name);
      ps.setString(2, owner == null ? "system" : owner);
      rs = ps.executeQuery();

      if (rs.next()) {
        int listId = rs.getInt(1);
        int maxElements = rs.getInt(4);

        list = new DBDistrList(listId, name, owner, maxElements, this);
        list = cache.add(list);
        return list;
      } else
        return null;

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  public Collection<DistrList> getDistrLists(String owner) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();
      if (owner != null) {
        ps = conn.prepareStatement(getSql("lists.get.by.owner"));
        ps.setString(1, owner == null ? "system" : owner);
      } else
        ps = conn.prepareStatement(getSql("lists.get.all"));

      rs = ps.executeQuery();

      Collection<DistrList> result = new ArrayList<DistrList>(20);

      while (rs.next()) {
        int listId = rs.getInt(1);
        String name = rs.getString(2);
        String listOwner = rs.getString(3);
        if (listOwner.equals("system"))
          listOwner = null;
        int maxElements = rs.getInt(4);

        DistrList list = new DBDistrList(listId, name, listOwner, maxElements, this);
        list = cache.add(list);
        result.add(list);
      }

      return result;

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  public boolean containsDistrList(String name, String owner) throws DataSourceException {
    return getDistrList(name, owner) != null;
  }

  public DistrList getDistrList(long id) throws DataSourceException {

    DistrList list = cache.get((int)id);
    if (list != null)
      return list;

    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("lists.get.by.id"));
      ps.setInt(1, (int)id);
      rs = ps.executeQuery();

      if (rs.next()) {
        int listId = rs.getInt(1);
        String name = rs.getString(2);
        String owner = rs.getString(3);
        if (owner.equals("system"))
          owner = null;
        int maxElements = rs.getInt(4);

        list = new DBDistrList(listId, name, owner, maxElements, this);
        list = cache.add(list);

        return list;
      } else
        return null;

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  void setMaxElements(int listId, int maxElements) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("lists.remove.by.name"));
      ps.setInt(1, maxElements);
      ps.setInt(2, listId);
      ps.execute();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  void addMember(int listId, String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("members.add"));
      ps.setInt(1, listId);
      ps.setString(2, address);
      ps.execute();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  boolean removeMember(int listId, String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("members.remove.by.address"));
      ps.setInt(1, listId);
      ps.setString(2, address);
      return ps.executeUpdate() > 0;
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  Collection<String> getMembers(int listId) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("members.get.by.group"));
      ps.setInt(1, listId);
      rs = ps.executeQuery();

      Collection<String> result = new ArrayList<String>(20);

      while (rs.next())
        result.add(rs.getString(1));

      return result;
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  boolean containsMember(int listId, String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("members.get.by.address"));
      ps.setInt(1, listId);
      ps.setString(2, address);
      rs = ps.executeQuery();

      return rs.next();            
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  void addSubmitter(int listId, String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("submitters.add"));
      ps.setInt(1, listId);
      ps.setString(2, address);
      ps.execute();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  boolean removeSubmitter(int listId, String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("submitters.remove.by.address"));
      ps.setInt(1, listId);
      ps.setString(2, address);
      return ps.executeUpdate() > 0;
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  Collection<String> getSubmitters(int listId) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("submitters.get.by.group"));
      ps.setInt(1, listId);
      rs = ps.executeQuery();

      Collection<String> result = new ArrayList<String>(20);

      while (rs.next())
        result.add(rs.getString(1));

      return result;
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  boolean containsSubmitter(int listId, String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("submitters.get.by.address"));
      ps.setInt(1, listId);
      ps.setString(2, address);
      rs = ps.executeQuery();

      return rs.next();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  void copyMembersAndSubmitters(int toList, DistrList fromList) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    PreparedStatement ps1 = null;
    try {
      conn = pool.getConnection();
      int size = 0;
      ps = conn.prepareStatement(getSql("submitters.add"));
      for (String submitter : fromList.submitters()) {
        ps.setInt(1, toList);
        ps.setString(2, submitter);
        ps.addBatch();
        if (size > 1000) {
          ps.executeBatch();
          size = 0;
        }
        size ++;
      }

      if (size > 0)
        ps.executeBatch();

      size = 0;
      ps1 = conn.prepareStatement(getSql("members.add"));
      for (String submitter : fromList.members()) {
        ps1.setInt(1, toList);
        ps1.setString(2, submitter);
        ps1.addBatch();
        if (size > 1000) {
          ps.executeBatch();
          size = 0;
        }
        size ++;
      }
      if (size > 0)
        ps1.executeBatch();

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps1, null);
      _close(null, ps, conn);
    }
  }

  public int size() throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("lists.count"));

      rs = ps.executeQuery();

      if (rs.next()) {
        return rs.getInt(1);
      } else
        return 0;

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  public void close() {
    pool.release();
  }
}
