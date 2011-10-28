package ru.novosoft.smsc.admin.archive_daemon;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.messages.*;
import ru.novosoft.smsc.admin.util.DBExportSettings;
import ru.novosoft.smsc.admin.util.ProgressObserver;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

@SuppressWarnings({"EmptyCatchBlock"})

/**
 * API для простмотра статистики смс
 * @author Aleksandr Khalitov
 */

public class ArchiveDaemon {

  private final static int MAX_SMS_FETCH_SIZE = 5000;

  private static final Logger logger = Logger.getLogger(ArchiveDaemon.class);

  private final ArchiveDaemonManager manager;

  public ArchiveDaemon(ArchiveDaemonManager manager) throws AdminException {
    this.manager = manager;
  }

  private Socket connect() throws AdminException, IOException {
    if (manager.getDaemonOnlineHost() == null)
      throw new ArchiveDaemonException("archive_daemon_offline");
    String host = manager.getSettings().getViewHost();
    int port = manager.getSettings().getViewPort();

    return new Socket(host, port);
  }

  /**
   * Возвращает статистику смс, удовлетворяющую запросу
   *
   * @param query    запрос
   * @param observer отслеживание прогресса
   * @return статистика смс
   * @throws AdminException ошибка извлечения статистики
   */
  public SmsSet getSmsSet(ArchiveMessageFilter query, ProgressObserver observer) throws AdminException {
    Socket socket = null;
    InputStream input = null;
    OutputStream output = null;

    SmsSet set = new SmsSet();
    set.setHasMore(false);
    int rowsMaximum = query.getRowsMaximum();
    observer.update(0, rowsMaximum);
    if (rowsMaximum == 0) return set;

    QueryMessage request = new QueryMessage(query);

    try {

      socket = connect();
      input = socket.getInputStream();
      output = new BufferedOutputStream(socket.getOutputStream());

      DaemonCommunicator communicator = new DaemonCommunicator(input, output);
      communicator.send(request);
      int toReceive = rowsMaximum < MAX_SMS_FETCH_SIZE ? rowsMaximum : MAX_SMS_FETCH_SIZE;

      communicator.send(new RsNxtMessage(toReceive));

      Message responce;
      boolean allSelected = false;
      int counter = 0;
      do {
        responce = communicator.receive();
        if (responce == null) throw new ArchiveDaemonException("invalid_response");

        switch (responce.getType()) {
          case Message.SMSC_BYTE_EMPTY_TYPE:
            allSelected = true;
            break;
          case Message.SMSC_BYTE_RSSMS_TYPE:
            set.addRow(((RsSmsMessage) responce).getSms());
            observer.update(++counter < rowsMaximum ? counter : rowsMaximum, rowsMaximum);
            if (--toReceive <= 0) {
              toReceive = rowsMaximum - set.getRowsCount();
              if (toReceive <= 0) {
                set.setHasMore(true);
                communicator.send(new EmptyMessage());
              } else {
                toReceive = (toReceive < MAX_SMS_FETCH_SIZE) ? toReceive : MAX_SMS_FETCH_SIZE;
                communicator.send(new RsNxtMessage(toReceive));
              }
            }
            break;
          case Message.SMSC_BYTE_ERROR_TYPE:
            logger.error("ArchiveDaemon communication error: " + ((ErrorMessage) responce).getError());
            throw new ArchiveDaemonException("error_returned");
          default:
            logger.error("Unknown response type: " + responce.getType());
            throw new ArchiveDaemonException("invalid_response");
        }
      } while (!allSelected);
      observer.update(rowsMaximum, rowsMaximum);

    } catch (IOException exc) {
      throw new ArchiveDaemonException("communication_error", exc);
    } finally {
      close(input, output, socket);
    }
    return set;
  }

  /**
   * Возвращает кол-во смс, удовлетворяющим запросу
   *
   * @param query запрос
   * @return кол-во смс
   * @throws AdminException ошибка извлечения статистики
   */
  public int getSmsCount(ArchiveMessageFilter query) throws AdminException {

    System.out.println("GET SMS COUNT");
    Socket socket = null;
    InputStream input = null;
    OutputStream output = null;

    int smsCount = 0;
    try {
      CountMessage request = new CountMessage(query);

      socket = connect();
      input = socket.getInputStream();
      output = socket.getOutputStream();

      DaemonCommunicator communicator = new DaemonCommunicator(input, output);
      communicator.send(request);

      Message responce = communicator.receive();
      if (responce == null)
        throw new ArchiveDaemonException("invalid_response");

      switch (responce.getType()) {
        case Message.SMSC_BYTE_EMPTY_TYPE:
          break;
        case Message.SMSC_BYTE_TOTAL_TYPE:
          smsCount = (int) ((TotalMessage) responce).getCount();
          break;
        case Message.SMSC_BYTE_ERROR_TYPE:
          logger.error("ArchiveDaemon communication error: " + ((ErrorMessage) responce).getError());
          throw new ArchiveDaemonException("error_returned");
        default:
          logger.error("Unknown response type: " + responce.getType());
          throw new ArchiveDaemonException("invalid_response");
      }
    } catch (IOException exc) {
      throw new ArchiveDaemonException("communication_error", exc);
    } finally {
      close(input, output, socket);
    }
    return smsCount;
  }

  private static void close(InputStream input, OutputStream output, Socket socket) {
    if (input != null) {
      try {
        input.close();
      } catch (IOException exc) {
      }
    }
    if (output != null) {
      try {
        output.close();
      } catch (IOException exc) {
      }
    }
    if (socket != null) {
      try {
        socket.close();
      } catch (IOException exc) {
      }
    }
  }


  private static final String mysqlDriver = "com.mysql.jdbc.Driver";
  private static final String oracleDriver = "oracle.jdbc.driver.OracleDriver";

  private Connection getConnection(DBExportSettings export) throws SQLException, ClassNotFoundException {
    switch (export.getDbType()) {
      case MYSQL:
        Class.forName(mysqlDriver);
        break;
      case ORACLE:
        Class.forName(oracleDriver);
    }
    Connection c = DriverManager.getConnection(export.getSource(), export.getUser(), export.getPass());
    c.setAutoCommit(false);
    return c;
  }

  protected final static String INSERT_VALUES =
      "	values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,   " +
          "   ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";


  protected final static String INSERT_OP_SQL = "INSERT INTO ";
  protected final static String DELETE_OP_SQL = "TRUNCATE TABLE ";

  protected final static String INSERT_FIELDS =
      "  (id,st,submit_time,valid_time,attempts,last_result, " +
          "   last_try_time,next_try_time,oa,da,dda,mr,svc_type,dr,br,src_msc,     " +
          "   src_imsi,src_sme_n,dst_msc,dst_imsi,dst_sme_n,route_id,svc_id,prty,  " +
          "   src_sme_id,dst_sme_id,msg_ref,seq_num,arc,body_len) ";


  public void clearTable(Connection conn, String tablePrefix, Date datePrefix) throws SQLException {
    SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy");
    PreparedStatement clearStmt = null;
    if (conn instanceof oracle.jdbc.OracleConnection && datePrefix == null)
      clearStmt = conn.prepareStatement(DELETE_OP_SQL + tablePrefix);
    else
      clearStmt = conn.prepareStatement("DELETE FROM " + tablePrefix +
          ((datePrefix != null) ? " WHERE LAST_TRY_TIME = '" + dateFormat.format(datePrefix) + "'" : ""));
    clearStmt.executeUpdate();
    conn.commit();
  }


  public synchronized void export(Date date, DBExportSettings export, final ProgressObserver observer) throws AdminException {
    Connection conn = null;
    PreparedStatement insertStmt = null;
    try {
      try {
        conn = getConnection(export);
      } catch (Exception e) {
        logger.error(e, e);
        throw new ArchiveDaemonException("cant_connect");
      }
      int inserted;

      String tablePrefix = export.getPrefix();
      String INSERT_SQL = INSERT_OP_SQL + tablePrefix + INSERT_FIELDS + INSERT_VALUES;
      try {
        insertStmt = conn.prepareStatement(INSERT_SQL);
      } catch (Exception e) {
        logger.error(e, e);
        throw new ArchiveDaemonException("internal_exception");
      }

      try {
        clearTable(conn, tablePrefix, date);
      } catch (Exception e) {
        logger.error(e, e);
        e.printStackTrace();            //todo
        throw new ArchiveDaemonException("cant_clean");
      }

      ArchiveMessageFilter query = new ArchiveMessageFilter();
      query.setRowsMaximum(600000);     //todo ask
      Calendar cal = new GregorianCalendar();
      Date fromDate, tillDate;
      cal.setTime(date);
      cal.set(Calendar.HOUR_OF_DAY, 0);
      cal.set(Calendar.MINUTE, 0);
      cal.set(Calendar.SECOND, 0);
      cal.set(Calendar.MILLISECOND, 0);
      for (int i = 0; i < 24; i++) {
        cal.set(Calendar.HOUR_OF_DAY, i);
        inserted = 0;
        final int _i = i;
        final ProgressObserver _observer = new ProgressObserver() {
          public void update(long current, long total) {
            int c = (int) (current);
            int t = (int) (total);
            observer.update((c * 100 / t / 24) + (_i * 100 / 24), 100);
          }
        };
        try {
          for (int j = 0; j < 60; j += 5) {
            cal.set(Calendar.MINUTE, j);
            cal.set(Calendar.SECOND, 0);
            cal.set(Calendar.MILLISECOND, 0);
            fromDate = cal.getTime();
            cal.set(Calendar.MINUTE, j + 4);
            cal.set(Calendar.SECOND, 59);
            cal.set(Calendar.MILLISECOND, 999);
            tillDate = cal.getTime();
            query.setFromDate(fromDate);
            query.setTillDate(tillDate);
            final int _j = i;
            SmsSet set = getSmsSet(query, new ProgressObserver() {
              public void update(long current, long total) {
                int c = (int) (current);
                int t = (int) (total);
                _observer.update((c * 100 / t / 12) + (_j * 100 / 12), 100);
              }
            });
            for (SmsRow row : set.getRowsList()) {
              setValues(insertStmt, row);
              inserted += insertStmt.executeUpdate();
            }
          }
          if (inserted > 0) conn.commit();
        } catch (Exception e) {
          logger.error(e, e);
          throw new ArchiveDaemonException("cant_insert");
        }
      }
    } catch (ArchiveDaemonException e) {
      rollbackConnection(conn);
      e.printStackTrace();                                                          //todo
      throw e;
    } catch (Exception e) {
      rollbackConnection(conn);
      logger.error(e, e);
      e.printStackTrace();                                                          //todo
      throw new ArchiveDaemonException("internal_exception");

    } finally {
      closeStatement(insertStmt);
      if (conn != null) {
        try {
          conn.close();
        } catch (Exception ignored) {
        }
      }
    }
  }

  private void rollbackConnection(Connection connection) {
    if (connection != null) {
      try {
        connection.rollback();
      } catch (Exception ignored) {
      }
    }
  }

  private void setValues(PreparedStatement stmt, SmsRow row) throws SQLException {
    int pos = 1;
    stmt.setLong(pos++, row.getId());
    stmt.setInt(pos++, row.getStatus().getCode());
    if (row.getSubmitTime() != null)
      stmt.setDate(pos++, new java.sql.Date(row.getSubmitTime().getTime()));
    else
      stmt.setDate(pos++, null);
    if (row.getValidTime() != null)
      stmt.setDate(pos++, new java.sql.Date(row.getValidTime().getTime()));
    else
      stmt.setDate(pos++, null);
    stmt.setInt(pos++, row.getAttempts());
    stmt.setInt(pos++, row.getLastResult());
    if (row.getLastTryTime() != null)
      stmt.setDate(pos++, new java.sql.Date(row.getLastTryTime().getTime()));
    else
      stmt.setDate(pos++, null);
    if (row.getNextTryTime() != null)
      stmt.setDate(pos++, new java.sql.Date(row.getNextTryTime().getTime()));
    else
      stmt.setDate(pos++, null);
    if (row.getOriginatingAddress() != null)
      stmt.setString(pos++, row.getOriginatingAddress().getSimpleAddress());
    else
      stmt.setString(pos++, null);
    if (row.getDestinationAddress() != null)
      stmt.setString(pos++, row.getDestinationAddress().getSimpleAddress());
    else
      stmt.setString(pos++, null);
    if (row.getDealiasedDestinationAddress() != null)
      stmt.setString(pos++, row.getDealiasedDestinationAddress().getSimpleAddress());
    else
      stmt.setString(pos++, null);
    stmt.setInt(pos++, row.getMessageReference());
    stmt.setString(pos++, row.getServiceType());
    stmt.setShort(pos++, row.getDeliveryReport());
    stmt.setShort(pos++, row.getBillingRecord());
    if (row.getOriginatingDescriptor() != null) {
      stmt.setString(pos++, row.getOriginatingDescriptor().getMsc());
      stmt.setString(pos++, row.getOriginatingDescriptor().getImsi());
      stmt.setLong(pos++, row.getOriginatingDescriptor().getSme());
    } else {
      stmt.setString(pos++, null);
      stmt.setString(pos++, null);
      stmt.setLong(pos++, 0);
    }
    if (row.getDestinationDescriptor() != null) {
      stmt.setString(pos++, row.getDestinationDescriptor().getMsc());
      stmt.setString(pos++, row.getDestinationDescriptor().getImsi());
      stmt.setLong(pos++, row.getDestinationDescriptor().getSme());
    } else {
      stmt.setString(pos++, null);
      stmt.setString(pos++, null);
      stmt.setLong(pos++, 0);
    }
    stmt.setString(pos++, row.getRouteId());
    stmt.setLong(pos++, row.getServiceId());
    stmt.setLong(pos++, row.getPriority());
    stmt.setString(pos++, row.getSrcSmeId());
    stmt.setString(pos++, row.getDstSmeId());
    stmt.setShort(pos++, row.getConcatMsgRef());
    stmt.setShort(pos++, row.getConcatSeqNum());
    stmt.setByte(pos++, row.getArc());
    stmt.setInt(pos, row.getBodyLen());
  }

  protected static void closeStatement(PreparedStatement stmt) {
    if (stmt != null) {
      try {
        stmt.close();
      } catch (Exception ignored) {
      }
    }
  }
}
