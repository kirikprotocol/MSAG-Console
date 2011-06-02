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

  private final TreeMap<String, Table> tables = new TreeMap<String, Table>(new Comparator<String>() {
    public int compare(String o1, String o2) {
      return o2.compareTo(o1);
    }
  });

  private String currentTable;


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
    return MessageFormat.format(pattern, values);
  }


  public SenderMessage loadSenderMessageById(final int id) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Load message: id="+id);
    }
    try{
      Table t = getTable(id);
      if(t == null) {
        return null;
      }
      SenderMessage r =  t.loadSenderMessageById(id);
      if(logger.isDebugEnabled()) {
        logger.debug("Sender DS: Load message resp: "+r);
      }
      return r;
    }catch (SQLException e){
      throw new DataSourceException(e);
    }
  }

  private final Lock createLock = new ReentrantLock();

  public void saveSenderMessage(final SenderMessage msg) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Save/Update message id="+(msg.isExists() ? msg.getId() : null)+" dest="+msg.getDestinationAddress());
    }
    try{
      Table t;
      if(msg.isExists()) {
        t = getTable(msg.getId());
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
    }catch (SQLException e){
      throw new DataSourceException(e);
    }
  }

  public void removeSenderMessage(final SenderMessage msg) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Remove message: id="+msg.getId());
    }
    try{
      Table t = getTable(msg.getId());
      if(t == null) {
        return;
      }
      t.removeSenderMessage(msg);
    }catch (SQLException e){
      throw new DataSourceException(e);
    }
  }

  public int updateMessageStatus(final long smppId, final int status) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Update status: smppId="+smppId+" status="+status);
    }
    try{
      Table t = getTable(null);
      if(t == null) {
        return 0;
      }
      int res = t.updateMessageStatus(smppId, status);
      if(res < 1) {  //record not fount
        t = getTableAfter(t.nameSuffix);
        res = (t == null) ? 0 : t.updateMessageStatus(smppId, status);
      }
      return res;
    }catch (SQLException e){
      throw new DataSourceException(e);
    }
  }

  public void updateMessageSmppId(final SenderMessage msg) throws DataSourceException {
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Update smppId="+msg.getSmppId()+" id="+msg.getId());
    }
    try{
      Table t = getTable(msg.getId());
      if(t == null) {
        return;
      }
      t.updateMessageSmppId(msg);
    }catch (SQLException e){
      throw new DataSourceException(e);
    }
  }

  public void release() {
    pool.release();
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
        logger.debug("Sender DS: new month became. Create table: "+_newTable);
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
          Map.Entry<String, Table> e = tables.lastEntry();  // самая старая таблица
          Table toRemove = e.getValue();
          toRemove.drop(conn);
          tables.remove(e.getKey());
          if(logger.isDebugEnabled()) {
            logger.debug("Sender DS: old table is removed: "+toRemove.nameSuffix);
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
      table = getTableByid(recordId);
    }
    if(logger.isDebugEnabled()) {
      logger.debug("Sender DS: Get table.Table for id="+recordId+": "+(table == null ? null : table.nameSuffix));
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
      logger.debug("Sender DS: Get table by date="+sDate+".Table is "+(table == null ? null : table.nameSuffix));
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
      logger.debug("Sender DS: Table after "+tableBefore+" is "+(result == null ? null : result.nameSuffix));
    }
    return result;
  }


  private Table getTableByid(int id) {
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


  /**
   * Абстрактная таблица. Хранит в себе sql выражения, а также максимальный идентификаторы.
   * Запросы к базе должны исполняться через неё.
   */
  @SuppressWarnings({"NullableProblems"})
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
        ps = conn.prepareStatement(statements.get("sender.message.load.max.id"));
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
        st = conn.prepareStatement(statements.get("sender.message.check.exist"));
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
        st = conn.prepareStatement(statements.get("sender.message.create.table"));
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
