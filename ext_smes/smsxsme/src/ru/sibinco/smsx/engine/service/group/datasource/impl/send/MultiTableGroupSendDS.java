package ru.sibinco.smsx.engine.service.group.datasource.impl.send;

import org.apache.log4j.Logger;
import ru.sibinco.smsx.engine.service.group.datasource.GroupSendDataSource;
import ru.sibinco.smsx.network.dbconnection.ConnectionPool;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;
import ru.sibinco.smsx.utils.DBDataSource;
import ru.sibinco.smsx.utils.DataSourceException;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.text.MessageFormat;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Aleksandr Khalitov
 */
@SuppressWarnings({"NullableProblems"})
public class MultiTableGroupSendDS extends DBDataSource implements GroupSendDataSource {

  private final ConnectionPool pool;

  private static final Logger logger = Logger.getLogger(MultiTableGroupSendDS.class);

  private final static int MAX_PREVIOUS_PERIODS = 2;

  private final static int PERIOD = Calendar.MONTH;

  private AtomicInteger maxId;

  private final SimpleDateFormat sdf;

  private final LinkedList<Table> tables = new LinkedList<Table>();


  public MultiTableGroupSendDS() throws DataSourceException {
    super(MultiTableGroupSendDS.class.getResourceAsStream("groupsend.multi.properties"), "");
    sdf = getDF(PERIOD);

    pool = ConnectionPoolFactory.createConnectionPool("groupsend.multi", Integer.MAX_VALUE, 60000);
    pool.init(1);
    initTables();
  }


  private SimpleDateFormat getDF(int period) {
    String pattern;
    switch (period) {
      case Calendar.HOUR_OF_DAY : pattern = "yyyyMMddHH"; break;
      case Calendar.DAY_OF_MONTH : pattern = "yyyyMMdd"; break;
      case Calendar.MONTH : pattern = "yyyyMM"; break;
      default: pattern = "yyyy";
    }
    return new SimpleDateFormat(pattern);
  }

  public void release() {
    pool.release();
  }

  private final Lock createLock = new ReentrantLock();

  public int insert(Collection<String> addresses) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender Group DS: Insert message size="+addresses.size());
    }
    Table t;
    Date date;
    int id;
    try{
      createLock.lock();
      date = new Date();
      id = maxId.incrementAndGet();
    }finally {
      createLock.unlock();
    }
    t = getTableByDate(date);
    t.insert(addresses, id);
    return id;
  }

  public Map<String, Integer> statuses(int id) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender Group DS: get statuses for id="+id);
    }
    Table t = getTableByMsgId(id);
    if(t == null) {
      return null;
    }
    return t.statuses(id);
  }

  public int updateStatus(long smppId, int status) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender Group DS: Update status: smppId="+smppId+" status="+status);
    }
    Table t = getCurrentTable();
    if(t == null) {
      return 0;
    }
    int res = t.updateStatus(smppId, status);
    if(res < 1) {  //record not fount
      t = getTableAfter(t.date);
      res = (t == null) ? 0 : t.updateStatus(smppId, status);
    }
    return res;
  }

  public int updateStatus(int id, String address, int status) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender Group DS: update status by address="+address+" for id="+id+". Status="+status);
    }
    Table t = getTableByMsgId(id);
    if(t == null) {
      return 0;
    }
    return t.updateStatus(id, address, status);
  }

  public int updateSmppId(int id, String address, long smppId) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender Group DS: update smppId by address="+address+" for id="+id+". SmppId="+smppId);
    }
    Table t = getTableByMsgId(id);
    if(t == null) {
      return 0;
    }
    return t.updateSmppId(id, address, smppId);
  }

  private static String formatByDate(String pattern, String ... values) {
    return (new MessageFormat(pattern)).format(values, new StringBuffer(), null).toString();
  }


  private TreeSet<String> getTableNames(Connection conn) throws DataSourceException, SQLException {
    TreeSet<String> _tables = new TreeSet<String>(new Comparator<String>() {
      public int compare(String o1, String o2) {
        return o2.compareTo(o1);
      }
    });
    PreparedStatement st = null;
    ResultSet rs = null;
    try{
      st = conn.prepareStatement(getSql("get.tables"));
      rs = st.executeQuery();
      while(rs.next()) {
        _tables.add(rs.getString(1));
      }
    }finally {
      _close(rs, st, null);
    }
    return _tables;
  }

  private void initTables() throws DataSourceException {
    Connection conn = null;
    try{
      conn = pool.getConnection();

      int max = 0;
      String prefix = getSql("table.prefix");
      for(String t : getTableNames(conn)) {
        Table table = new Table(t.substring(prefix.length()), conn, false);
        tables.add(table);
        if(table.table_maxId > max) {
          max = table.table_maxId;
        }
      }
      maxId = new AtomicInteger(max);

      removeOldTable(conn);

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    }finally {
      _close(null, null, conn);
    }
  }

  private void removeOldTable(Connection conn) throws SQLException {
    while(tables.size() > MAX_PREVIOUS_PERIODS + 1 ) {
      Table toRemove  = tables.getLast();  // самая старая таблица
      toRemove.drop(conn);
      tables.removeLast();
      if(logger.isDebugEnabled()) {
        logger.debug("Sender DS: old table is removed: "+toRemove.date);
      }
    }
  }



  /**
   * Возвращает таблицу, где может находиться запись с recordId.
   * Если recordId == null, вернёт текущую таблицу.
   * При необходимости создаёт новые таблицы и чистит устаревшие
   * @param msgId идентификатор записи
   * @return таблица
   */
  private synchronized Table getTableByMsgId(Integer msgId) {
    Table cand = null;
    for(Table t : tables) {       // (список отсортирован по убыванию даты)
      int m = t.table_maxId;
      if(msgId > m) {
        break;
      }else {
        cand = t;
      }
    }
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Get table.Table for id="+msgId+": "+(cand == null ? null : cand.date));
    }
    return cand;
  }

  /**
   * Возвращает текущую таблицу
   * @return таблица
   */
  private synchronized Table getCurrentTable() {
    Table table = tables.getFirst();
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Get current table. Table: "+(table == null ? null : table.date));
    }
    return table;
  }

  /**
   * Возвращает таблицу по дате.
   * Если recordId == null, вернёт текущую таблицу.
   * При необходимости создаёт новые таблицы и чистит устаревшие
   * @param date дата
   * @return таблица
   * @throws DataSourceException ошибка при создании или удалении таблиц
   */
  private synchronized Table getTableByDate(Date date) throws DataSourceException {

    String sDate = sdf.format(date);

    Table table = null;

    if (tables.getFirst().date.compareTo(sDate) < 0) {

      if(logger.isDebugEnabled()) {
        logger.debug("Sender DS: new month became. Create table: "+sDate);
      }

      Connection conn = null;
      try{
        conn = pool.getConnection();
        table =  new Table(sDate, conn, true);
        table.table_maxId = maxId.get();
        tables.addFirst(table);
        removeOldTable(conn);
      }catch (SQLException e){
        pool.invalidateConnection(conn);
        throw new DataSourceException(e);
      } finally {
        _close(null, null, conn);
      }
    } else {
      for(Table t : tables) {
        if(t.date.equals(sDate)) {
          table =  t;
          break;
        }
      }
    }

    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Get table by date="+sDate+".Table is "+(table == null ? null : table.date));
    }
    return table;
  }

  /**
   * Возвращает таблицу с наибольшой датой, дата которой меньше данной.
   * При необходимости создаёт новые таблицы и чистит устаревшие
   * @param tableBefore данная таблица
   * @return таблица
   */
  private synchronized Table getTableAfter(String tableBefore) {

    for (Table t : tables) {
      if (t.date.compareTo(tableBefore) < 0)
        return t;
    }


    return null;
  }


  private class Table {

    private final String date;
    private final Map<String, String> statements = new HashMap<String, String>(10);

    private int table_maxId;

    private Table(String date, Connection conn, boolean create) throws SQLException{
      this.date = date;
      prepareStatements();
      initTable(conn, create);
    }

    private int loadMaxId(Connection conn) throws SQLException {
      PreparedStatement ps = null;
      ResultSet rs = null;
      try {
        ps = conn.prepareStatement(statements.get("load.max.id"));
        rs = ps.executeQuery();
        return (rs.next()) ? rs.getInt(1) : 0;
      } finally {
        _close(rs, ps, null);
      }
    }

    private void initTable(Connection conn, boolean create) throws SQLException {
      if(create) {
        PreparedStatement st = null;
        try{
          st = conn.prepareStatement(statements.get("create.table"));
          st.executeUpdate();
        }finally {
          if(st != null) {
            _close(null, st, null);
          }
        }
      }
      table_maxId = loadMaxId(conn);
    }


    private void prepareStatements() {
      for (Map.Entry<String, String> e : getSqls().entrySet()) {
        statements.put(e.getKey(), formatByDate(e.getValue(), date));
      }
    }



    private void drop(Connection conn) throws SQLException {
      PreparedStatement st = null;
      try{
        st = conn.prepareStatement(statements.get("drop.table"));
        st.executeUpdate();
      }finally {
        if(st != null) {
          _close(null, st, null);
        }
      }
    }



    public void insert(Collection<String> addresses, int id) throws DataSourceException {
      Connection conn = null;
      PreparedStatement ps = null;
      try {
        conn = pool.getConnection();
        if (conn.getAutoCommit())
          conn.setAutoCommit(false);

        for (String address : addresses) {
          ps = conn.prepareStatement(statements.get("insert"));
          ps.setInt(1, id);
          ps.setString(2, address);
          ps.executeUpdate();
        }

        conn.commit();
        conn.setAutoCommit(true);

        table_maxId = id;
      } catch (SQLException e) {
        pool.invalidateConnection(conn);
        throw new DataSourceException(e);
      } finally {
        _close(null, ps, conn);
      }
    }

    public Map<String, Integer> statuses(int id) throws DataSourceException {
      Connection conn = null;
      PreparedStatement ps = null;
      ResultSet rs = null;
      try {
        conn = pool.getConnection();
        ps = conn.prepareStatement(statements.get("statuses"));
        ps.setInt(1, id);
        rs = ps.executeQuery();

        if (rs != null) {
          Map<String, Integer> result = new HashMap<String, Integer>(10);
          while (rs.next())
            result.put(rs.getString(1), rs.getInt(2));
          return result;
        }

        return null;

      } catch (SQLException e) {
        pool.invalidateConnection(conn);
        throw new DataSourceException(e);
      } finally {
        _close(rs, ps, conn);
      }
    }

    public int updateStatus(long smppId, int status) throws DataSourceException {
      Connection conn = null;
      PreparedStatement ps = null;

      try {
        conn = pool.getConnection();

        ps = conn.prepareStatement(statements.get("update.status"));

        ps.setInt(1, status);
        ps.setLong(2, smppId);

        return ps.executeUpdate();

      } catch (SQLException e) {
        pool.invalidateConnection(conn);
        throw new DataSourceException(e);
      } finally {
        _close(null, ps, conn);
      }
    }

    public int updateStatus(int id, String address, int status) throws DataSourceException {
      Connection conn = null;
      PreparedStatement ps = null;

      try {
        conn = pool.getConnection();

        ps = conn.prepareStatement(statements.get("update.status.by.id"));

        ps.setInt(1, status);
        ps.setInt(2, id);
        ps.setString(3, address);

        return ps.executeUpdate();

      } catch (SQLException e) {
        pool.invalidateConnection(conn);
        throw new DataSourceException(e);
      } finally {
        _close(null, ps, conn);
      }
    }

    public int updateSmppId(int id, String address, long smppId) throws DataSourceException {
      Connection conn = null;
      PreparedStatement ps = null;

      try {
        conn = pool.getConnection();

        ps = conn.prepareStatement(statements.get("update.smpp.id"));

        ps.setLong(1, smppId);
        ps.setInt(2, id);
        ps.setString(3, address);

        return ps.executeUpdate();

      } catch (SQLException e) {
        pool.invalidateConnection(conn);
        throw new DataSourceException(e);
      } finally {
        _close(null, ps, conn);
      }
    }

  }
}
