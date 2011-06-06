package ru.sibinco.smsx.engine.service.sender.datasource;

import org.apache.log4j.Logger;
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
 * author: Aleksandr Khalitov
 *
 */

@SuppressWarnings({"NullableProblems"})
public class MultiTableSenderDS extends DBDataSource implements SenderDataSource {

  private static final Logger logger = Logger.getLogger(MultiTableSenderDS.class);

  private final ConnectionPool pool;

  private final static int MAX_PREVIOUS_PERIODS = 2;

  private final static int PERIOD = Calendar.MONTH;

  private AtomicInteger maxId;

  private final SimpleDateFormat sdf;

  private final LinkedList<Table> tables = new LinkedList<Table>();


  public MultiTableSenderDS() throws DataSourceException {
    super(DBSenderDataSource.class.getResourceAsStream("sender.multi.properties"), "");
    sdf = getDF(PERIOD);
    pool = ConnectionPoolFactory.createConnectionPool("sender.multi", Integer.MAX_VALUE, 60000);
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

  private static String formatByDate(String pattern, String ... values) {
    return (new MessageFormat(pattern)).format(values, new StringBuffer(), null).toString();
  }


  public SenderMessage loadSenderMessageById(final int id) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Load message: id="+id);
    }
    Table t = getTableByMsgId(id);
    if(t == null) {
      return null;
    }
    SenderMessage r =  t.loadSenderMessageById(id);
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Load message resp: "+r);
    }
    return r;
  }

  private final Lock createLock = new ReentrantLock();

  public void saveSenderMessage(final SenderMessage msg) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Save/Update message id="+(msg.isExists() ? msg.getId() : null)+" dest="+msg.getDestinationAddress());
    }
    Table t;
    if(msg.isExists()) {
      t = getTableByMsgId(msg.getId());
      if(t == null) {
        return;
      }
      t.saveSenderMessage(msg, true);
    }else {
      Date date;
      try{
        createLock.lock();
        date = new Date();
        msg.setId(maxId.incrementAndGet());
      }finally {
        createLock.unlock();
      }
      t = getTableByDate(date);
      t.saveSenderMessage(msg, false);
    }
  }

  public void removeSenderMessage(final SenderMessage msg) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Remove message: id="+msg.getId());
    }
    Table t = getTableByMsgId(msg.getId());
    if(t == null) {
      return;
    }
    t.removeSenderMessage(msg);
  }

  public int updateMessageStatus(final long smppId, final int status) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Update status: smppId="+smppId+" status="+status);
    }

    Table t = getCurrentTable();
    if(t == null) {
      return 0;
    }
    int res = t.updateMessageStatus(smppId, status);
    if(res < 1) {  //record not fount
      t = getTableAfter(t.date);
      res = (t == null) ? 0 : t.updateMessageStatus(smppId, status);
    }
    return res;
  }

  public void updateMessageSmppId(final SenderMessage msg) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Update smppId="+msg.getSmppId()+" id="+msg.getId());
    }
    Table t = getTableByMsgId(msg.getId());
    if(t == null) {
      return;
    }
    t.updateMessageSmppId(msg);
  }

  public void release() {
    pool.release();
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


  /**
   * Абстрактная таблица. Хранит в себе sql выражения, а также максимальный идентификаторы.
   * Запросы к базе должны исполняться через неё.
   */
  @SuppressWarnings({"NullableProblems"})
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
        ps = conn.prepareStatement(statements.get("sender.message.load.max.id"));
        rs = ps.executeQuery();
        return (rs.next()) ? rs.getInt(1) : 0;
      } finally {
        _close(rs, ps, null);
      }
    }

    private void initTable(Connection conn, boolean create) throws SQLException {
      if (create) {
        PreparedStatement st = null;
        try{
          st = conn.prepareStatement(statements.get("sender.message.create.table"));
          st.executeUpdate();
        }finally {
          _close(null, st, null);
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
        st = conn.prepareStatement(statements.get("sender.message.drop.table"));
        st.executeUpdate();
      }finally {
        if(st != null) {
          _close(null, st, null);
        }
      }
    }

    public SenderMessage loadSenderMessageById(int id) throws DataSourceException {
      Connection conn = null;
      PreparedStatement ps = null;
      ResultSet rs = null;
      try {
        conn = pool.getConnection();
        ps = conn.prepareStatement(statements.get("sender.message.load.by.id"));
        ps.setInt(1, id);
        rs = ps.executeQuery();

        if (rs != null && rs.next()) {
          final SenderMessage msg = new SenderMessage(rs.getInt(1));
          msg.setSmppStatus(rs.getInt(2));
          msg.setStatus(rs.getInt(3));
          return msg;
        }

        return null;

      } catch (SQLException e) {
        pool.invalidateConnection(conn);
        throw new DataSourceException(e);
      } finally {
        _close(rs, ps, conn);
      }

    }

    public void saveSenderMessage(SenderMessage msg, boolean exist) throws DataSourceException {

      Connection conn = null;
      PreparedStatement ps = null;

      try {
        conn = pool.getConnection();

        ps = conn.prepareStatement(statements.get(exist ? "sender.message.update" : "sender.message.insert"));

        if(!exist) {
          table_maxId = msg.getId();
        }

        ps.setInt(1, msg.getSmppStatus());
        ps.setInt(2, msg.getStatus());
        ps.setLong(3, msg.getSmppId());
        ps.setInt(4, msg.getId());

        ps.executeUpdate();

      } catch (SQLException e) {
        pool.invalidateConnection(conn);
        throw new DataSourceException(e);
      } finally {
        _close(null, ps, conn);
      }
    }

    public void removeSenderMessage(SenderMessage msg) throws DataSourceException {
      if (!msg.isExists())
        return;

      Connection conn = null;
      PreparedStatement ps = null;

      try {
        conn = pool.getConnection();

        ps = conn.prepareStatement(statements.get("sender.message.remove"));

        ps.setInt(1, msg.getId());

        ps.executeUpdate();

      } catch (SQLException e) {
        pool.invalidateConnection(conn);
        throw new DataSourceException(e);
      } finally {
        _close(null, ps, conn);
      }
    }

    public int updateMessageStatus(long smppId, int status) throws DataSourceException {
      Connection conn = null;
      PreparedStatement ps = null;

      try {
        conn = pool.getConnection();

        ps = conn.prepareStatement(statements.get("sender.message.update.by.smpp.id"));

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

    public void updateMessageSmppId(SenderMessage msg) throws DataSourceException {
      if (!msg.isExists())
        return;

      Connection conn = null;
      PreparedStatement ps = null;

      try {
        conn = pool.getConnection();

        ps = conn.prepareStatement(statements.get("sender.message.update.smpp.id"));

        ps.setLong(1, msg.getSmppId());
        ps.setInt(2, msg.getId());

        ps.executeUpdate();

      } catch (SQLException e) {
        pool.invalidateConnection(conn);
        throw new DataSourceException(e);
      } finally {
        _close(null, ps, conn);
      }
    }
  }


}
