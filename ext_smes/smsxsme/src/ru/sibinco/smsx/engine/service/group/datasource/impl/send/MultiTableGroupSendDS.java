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

  private final TreeMap<String, Table> tables = new TreeMap<String, Table>(new Comparator<String>() {
    public int compare(String o1, String o2) {
      return o2.compareTo(o1);
    }
  });

  private String currentTable;



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
    try{
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
    }catch (SQLException e){
      throw new DataSourceException(e);
    }
  }

  public Map<String, Integer> statuses(int id) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender Group DS: get statuses for id="+id);
    }
    try{
      Table t = getTable(id);
      if(t == null) {
        return null;
      }
      return t.statuses(id);
    }catch (SQLException e) {
      throw new DataSourceException(e);
    }
  }

  public int updateStatus(long smppId, int status) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender Group DS: Update status: smppId="+smppId+" status="+status);
    }
    try{
      Table t = getTable(null);
      if(t == null) {
        return 0;
      }
      int res = t.updateStatus(smppId, status);
      if(res < 1) {  //record not fount
        t = getTableAfter(t.nameSuffix);
        res = (t == null) ? 0 : t.updateStatus(smppId, status);
      }
      return res;
    }catch (SQLException e){
      throw new DataSourceException(e);
    }
  }

  public int updateStatus(int id, String address, int status) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender Group DS: update status by address="+address+" for id="+id+". Status="+status);
    }
    try{
      Table t = getTable(id);
      if(t == null) {
        return 0;
      }
      return t.updateStatus(id, address, status);
    }catch (SQLException e) {
      throw new DataSourceException(e);
    }
  }

  public int updateSmppId(int id, String address, long smppId) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender Group DS: update smppId by address="+address+" for id="+id+". SmppId="+smppId);
    }
    try{
      Table t = getTable(id);
      if(t == null) {
        return 0;
      }
      return t.updateSmppId(id, address, smppId);
    }catch (SQLException e) {
      throw new DataSourceException(e);
    }
  }

  private static String formatByDate(String pattern, String ... values) {
    return (new MessageFormat(pattern)).format(values, new StringBuffer(), null).toString();
  }

  private void initTables() throws DataSourceException {
    Calendar c = Calendar.getInstance();
    Connection conn = null;
    try{
      conn = pool.getConnection();
      currentTable = sdf.format(c.getTime());
      addTable(currentTable, conn, false);

      String nameSuffix;

      for(int i=0; i < MAX_PREVIOUS_PERIODS;i++) {
        c.add(PERIOD, -1);
        nameSuffix = sdf.format(c.getTime());
        addTable(nameSuffix, conn, true);
      }

      for(int i=0; i < 12 - MAX_PREVIOUS_PERIODS;i++) {   // удаление более старых
        c.add(PERIOD, -1);
        nameSuffix = sdf.format(c.getTime());
        new Table(nameSuffix).drop(conn);
      }

      int max = 0;
      for(Table t : tables.values()) {
        if(t.table_maxId > max) {
          max = t.table_maxId;
        }
      }
      maxId = new AtomicInteger(max);

    } catch (SQLException e) {
      throw new DataSourceException(e);
    }finally {
      _close(null, null, conn);
    }
  }

  private void switchTables() throws SQLException {
    Calendar c = Calendar.getInstance();
    String _newTable = sdf.format(c.getTime());

    if(!_newTable.equals(currentTable)) {
      if(logger.isDebugEnabled()) {
        logger.debug("Sender Group DS: new month became. Create table: "+_newTable);
      }

      Connection conn = null;
      Table nT =  new Table(_newTable);
      try{
        conn = pool.getConnection();
        nT.initTable(conn);
        nT.table_maxId = maxId.get();
        tables.put(_newTable, nT);
        currentTable = _newTable;
        if(tables.size() > MAX_PREVIOUS_PERIODS + 1 ) {
          String key = tables.lastKey();
          Table toRemove  = tables.get(key);  // самая старая таблица
          toRemove.drop(conn);
          tables.remove(key);
          if(logger.isDebugEnabled()) {
            logger.debug("Sender Group DS: old table is removed: "+toRemove.nameSuffix);
          }
        }
      }finally {
        _close(null, null, conn);
      }
    }
  }

  private void addTable(String nameSuffix, Connection conn, boolean checkExist) throws SQLException {
    Table t = new Table(nameSuffix);
    if(checkExist && !t.isExist(conn)) {
      return;
    }
    t.initTable(conn);
    tables.put(nameSuffix, t);
  }


  /**
   * Возвращает таблицу, где может находиться запись с recordId.
   * Если recordId == null, вернёт текущую таблицу.
   * При необходимости создаёт новые таблицы и чистит устаревшие
   * @param recordId идентификатор записи
   * @return таблица
   * @throws SQLException ошибка при создании или удалении таблиц
   */
  private synchronized Table getTable(Integer recordId) throws SQLException {

    switchTables();

    Table table;
    if(recordId == null) {
      table = tables.get(currentTable);
    }else {
      table = _getTableById(recordId);
    }
    if(logger.isDebugEnabled()) {
      logger.debug("Sender Group DS: Get table.Table for id="+recordId+": "+(table == null ? null : table.nameSuffix));
    }
    return table;
  }


  /**
   * Возвращает таблицу по дате.
   * Если recordId == null, вернёт текущую таблицу.
   * При необходимости создаёт новые таблицы и чистит устаревшие
   * @param date дата
   * @return таблица
   * @throws SQLException ошибка при создании или удалении таблиц
   */
  private synchronized Table getTableByDate(Date date) throws SQLException {

    switchTables();

    String sDate = date == null ? null : sdf.format(date);
    Table table = sDate == null ? tables.get(currentTable) : tables.get(sDate);
    if(logger.isDebugEnabled()) {
      logger.debug("Sender Group DS: Get table by date="+sDate+".Table is "+(table == null ? null : table.nameSuffix));
    }
    return table;
  }

  /**
   * Возвращает таблицу с наибольшой датой, дата которой меньше данной.
   * При необходимости создаёт новые таблицы и чистит устаревшие
   * @param tableBefore данная таблица
   * @return таблица
   * @throws SQLException ошибка при создании или удалении таблиц
   */
  private synchronized Table getTableAfter(String tableBefore) throws SQLException {

    switchTables();

    boolean foundBefore = false;
    Table result = null;
    for(Map.Entry<String, Table> e : tables.entrySet()) {
      if(foundBefore) {
        result = e.getValue();
        break;
      }
      if(e.getKey().equals(tableBefore)) {
        foundBefore = true;
      }
    }
    if(logger.isDebugEnabled()) {
      logger.debug("Sender Group DS: Table after "+tableBefore+" is "+(result == null ? null : result.nameSuffix));
    }
    return result;
  }


  private Table _getTableById(int id) {
    Table cand = null;
    for(Table t : tables.values()) {       // (список отсортирован по убыванию даты)
      int m = t.table_maxId;
      if(id > m) {
        return cand;
      }else {
        cand = t;
      }
    }
    return cand;
  }


  private class Table {

    private final String nameSuffix;
    private final Map<String, String> statements = new HashMap<String, String>(10);

    private int table_maxId;

    private Table(String nameSuffix) {
      this.nameSuffix = nameSuffix;
      prepareStatements();
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

    private boolean isExist(Connection conn) throws SQLException {
      PreparedStatement st = null;
      ResultSet rs = null;
      try{
        st = conn.prepareStatement(statements.get("check.exist"));
        rs = st.executeQuery();
        return rs.next();
      }finally {
        if(st != null) {
          _close(rs, st, null);
        }
      }
    }

    private void initTable(Connection conn) throws SQLException {
      PreparedStatement st = null;
      try{
        st = conn.prepareStatement(statements.get("create.table"));
        st.executeUpdate();
      }finally {
        if(st != null) {
          _close(null, st, null);
        }
      }
      table_maxId = loadMaxId(conn);
    }


    private void prepareStatements() {
      for (Map.Entry<String, String> e : getSqls().entrySet()) {
        statements.put(e.getKey(), formatByDate(e.getValue(), nameSuffix));
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
