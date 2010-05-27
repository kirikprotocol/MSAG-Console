package ru.novosoft.smsc.infosme.backend.siebel.impl;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.infosme.backend.siebel.*;
import ru.sibinco.util.conpool.ConnectionPool;

import java.io.IOException;
import java.io.InputStream;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.Date;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

/**
 * author: alkhal
 */
public class SiebelDataProviderImpl implements SiebelDataProvider {

  private static final Logger logger = Logger.getLogger(SiebelDataProviderImpl.class);

  private static final int FETCH_SIZE = 1000;

  private ConnectionPool pool;

  private final Properties sql;


  public SiebelDataProviderImpl() throws SiebelException {
    try {
      InputStream is = this.getClass().getResourceAsStream("db.properties");
      sql = new Properties();
      try {
        sql.load(is);
      } finally {
        try {
          if (is != null)
            is.close();
        } catch (IOException e1) {
          logger.error("Can't close stream", e1);
        }
      }
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  /**
   * @noinspection EmptyCatchBlock
   */
  public void connect(Properties props) throws SiebelException {
    if (pool != null) {
      try {
        pool.shutdown();
      } catch (SQLException e) {
      }
    }
    try {
      pool = new ConnectionPool(props);
      shutdowned = false;
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  public boolean isShutdowned() {
    return shutdowned;
  }

  private String getSql(java.lang.String string) {
    return sql.getProperty(string);
  }

  public SiebelMessage getMessage(String clcId) throws SiebelException {
    if (clcId == null) {
      throw new SiebelException("Argument is null");
    }
    SiebelMessage siebelMessage = null;
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("message.get"));
      prepStatement.setFetchSize(FETCH_SIZE);
      prepStatement.setString(1, clcId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        siebelMessage = new SiebelMessage();
        siebelMessage.setClcId(clcId);
        siebelMessage.setCreated(new Date(sqlResult.getTimestamp(getSql("message.created")).getTime()));
        siebelMessage.setLastUpd(new Date(sqlResult.getTimestamp(getSql("message.last.upd")).getTime()));
        siebelMessage.setMessage(sqlResult.getString(getSql("message.message")));
        String ms = sqlResult.getString(getSql("message.message.state"));
        if (ms != null) {
          siebelMessage.setMessageState(SiebelMessage.State.valueOf(ms));
        }
        siebelMessage.setMsisdn(sqlResult.getString(getSql("message.msisdn")));
        siebelMessage.setSmscCode(sqlResult.getString(getSql("message.smsc.stat.code")));
        siebelMessage.setSmscValue(sqlResult.getString(getSql("message.smsc.stat.val")));
        siebelMessage.setWaveId(sqlResult.getString(getSql("message.wave.id")));
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get siebelMessage " + siebelMessage);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SiebelMessage from the dataBase with clcId: " + clcId, exc);
      throw new SiebelException("Unable to get SiebelMessage from the dataBase with clcId: " + clcId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return siebelMessage;
  }


  public ResultSet getMessages(String waveId) throws SiebelException {
    if (waveId == null) {
      throw new SiebelException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("message.list"));
      prepStatement.setFetchSize(FETCH_SIZE);
      prepStatement.setString(1, waveId);

      sqlResult = prepStatement.executeQuery();

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get list of messages");
      }
    } catch (Throwable exc) {
      logger.error("Unable to get list of messages from the dataBase", exc);
      closeConn(connection, prepStatement, sqlResult);
      throw new SiebelException("Unable to get list of messages from the dataBase", exc);
    }
    return new SiebelMessagesResultSet(sqlResult, connection, prepStatement, sql);
  }

  public boolean containsUnfinished(String waveId) throws SiebelException {
    if (waveId == null) {
      throw new SiebelException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("messages.get.unfinished"));
      prepStatement.setFetchSize(1);
      prepStatement.setString(1, waveId);

      sqlResult = prepStatement.executeQuery();

      return sqlResult.next();
    } catch (Throwable exc) {
      logger.error("Unable to get list of messages from the dataBase", exc);
      throw new SiebelException("Unable to get list of messages from the dataBase", exc);
    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
  }

  public SiebelTask getTask(String waveId) throws SiebelException {
    if (waveId == null) {
      throw new SiebelException("Argument is null");
    }
    SiebelTask siebelTask = null;
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("task.get"));
      prepStatement.setFetchSize(FETCH_SIZE);
      prepStatement.setString(1, waveId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        siebelTask = new SiebelTask();
        siebelTask = new SiebelTask();
        siebelTask.setLastUpdate(new java.util.Date(sqlResult.getTimestamp(getSql("task.last.upd")).getTime()));
        siebelTask.setBeep(sqlResult.getString(getSql("task.beep")).equals("Y"));
        siebelTask.setCampaignId(sqlResult.getString(getSql("task.campaign.id")));
        siebelTask.setCreated(new java.util.Date(sqlResult.getTimestamp(getSql("task.created")).getTime()));
        siebelTask.setStatus(SiebelTask.Status.valueOf(
            sqlResult.getString(getSql("task.ctrl.status"))));
        siebelTask.setExpPeriod(new Integer(sqlResult.getInt(getSql("task.exp.period"))));
        siebelTask.setFlash(sqlResult.getString(getSql("task.flash")).equals("Y"));
        siebelTask.setPriority(sqlResult.getInt(getSql("task.priority")));
        siebelTask.setSave(sqlResult.getString(getSql("task.save")).equals("Y"));
        siebelTask.setWaveId(waveId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get siebelTask " + siebelTask);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SiebelTask from the dataBase with waveId: " + waveId, exc);
      throw new SiebelException("Unable to get SiebelTask from the dataBase with waveId: " + waveId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return siebelTask;
  }

  public ResultSet getTasksToUpdate() throws SiebelException {

    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("task.list.update"));
      prepStatement.setFetchSize(FETCH_SIZE);

      sqlResult = prepStatement.executeQuery();

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get list of task");
      }
    } catch (Throwable exc) {
      logger.error("Unable to get list of tasks from the dataBase", exc);
      closeConn(connection, prepStatement, sqlResult);
      throw new SiebelException("Unable to get list of tasks from the dataBase", exc);
    }
    return new SiebelTasksResultSet(sqlResult, connection, prepStatement, sql);
  }

  public ResultSet getTasks() throws SiebelException {
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("task.list"));
      prepStatement.setFetchSize(FETCH_SIZE);

      sqlResult = prepStatement.executeQuery();

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get list of messages");
      }
    } catch (Throwable exc) {
      logger.error("Unable to get list of tasks from the dataBase", exc);
      closeConn(connection, prepStatement, sqlResult);
      throw new SiebelException("Unable to get list of tasks from the dataBase", exc);
    }
    return new SiebelTasksResultSet(sqlResult, connection, prepStatement, sql);
  }

  public void setTaskStatus(String waveId, SiebelTask.Status status) throws SiebelException {
    if (status == null || waveId == null) {
      throw new SiebelException("Some arguments are null");
    }

    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnection();
      prepStatement = connection.prepareStatement(getSql("task.set.status"));
      prepStatement.setString(1, status.toString());
      prepStatement.setTimestamp(2, new Timestamp(System.currentTimeMillis()));
      prepStatement.setString(3, waveId);
      int rowsUpdated = prepStatement.executeUpdate();

      if (rowsUpdated == 0) {
        throw new SiebelException("SiebelTask not found for waveId=" + waveId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful setting ctrl_status=" + status + " for waveId=" + waveId);
      }
    } catch (SQLException exc) {
      logger.error("Unable to set ctrl_status=" + status + " for waveId=" + waveId);
      throw new SiebelException("Unable to set ctrl_status=" + status + " for waveId=" + waveId, exc);
    } finally {
      closeConn(connection, prepStatement, null);
    }
  }

  public SiebelTask.Status getTaskStatus(String waveId) throws SiebelException {
    if (waveId == null) {
      throw new SiebelException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;
    SiebelTask.Status status = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("task.get.status"));
      prepStatement.setFetchSize(FETCH_SIZE);
      prepStatement.setString(1, waveId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        String st =
            sqlResult.getString(getSql("task.ctrl.status"));
        if (st != null) {
          status = SiebelTask.Status.valueOf(st);
        }
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get SmsMailParam's status " + status);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SmsMailParam's status with waveId: " + waveId, exc);
      throw new SiebelException("Unable to get SmsMailParam's status with waveId: " + waveId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return status;
  }

  public void setMessageState(String clcId, SiebelMessage.State state) throws SiebelException {
    if (state == null || clcId == null) {
      throw new SiebelException("Some arguments are null");
    }

    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnection();
      prepStatement = connection.prepareStatement(getSql("message.set.status"));
      prepStatement.setString(1, state.toString());
      prepStatement.setTimestamp(2, new Timestamp(System.currentTimeMillis()));
      prepStatement.setString(3, clcId);
      int rowsUpdated = prepStatement.executeUpdate();

      if (rowsUpdated == 0) {
        throw new SiebelException("SiebelMessage not found for clcId=" + clcId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful setting state=" + state + " for clcId=" + clcId);
      }
    } catch (SQLException exc) {
      logger.error("Unable to set state=" + state + " for clcId=" + clcId);
      throw new SiebelException("Unable to set cstates=" + state + " for clcId=" + clcId, exc);
    } finally {
      closeConn(connection, prepStatement, null);
    }
  }

  public SiebelMessage.State getMessageState(String clcId) throws SiebelException {
    if (clcId == null) {
      throw new SiebelException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;
    SiebelMessage.State state = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("message.get.status"));
      prepStatement.setFetchSize(FETCH_SIZE);
      prepStatement.setString(1, clcId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        String st =
            sqlResult.getString(getSql("message.message.state"));
        if (st != null) {
          state = SiebelMessage.State.valueOf(st);
        }
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get SmsMailParam's state " + state);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SiebelMessage state with clcId: " + clcId, exc);
      throw new SiebelException("Unable to get SiebelMessage state with clcId: " + clcId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return state;
  }

  /**
   * @noinspection EmptyCatchBlock
   */
  public void updateDeliveryStates(Map deliveryStates) throws SiebelException {
    if (deliveryStates == null) {
      throw new SiebelException("Some arguments are null");
    }
    if (deliveryStates.isEmpty()) {
      return;
    }

    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnection();
      boolean autoCommit = connection.getAutoCommit();
      connection.setAutoCommit(false);
      try {
        Iterator i = deliveryStates.entrySet().iterator();
        int count = 0;
        while (i.hasNext()) {
          if (prepStatement == null) {
            prepStatement = connection.prepareStatement(getSql("message.update.delivered"));
          }
          Map.Entry e = (Map.Entry) i.next();
          String clcId = (String) e.getKey();
          SiebelMessage.DeliveryState deliverySt = (SiebelMessage.DeliveryState) e.getValue();
          prepStatement.setString(1, deliverySt.getSmppCode());
          prepStatement.setString(2, deliverySt.getSmppCodeDescription());
          prepStatement.setString(3, deliverySt.getState().toString());
          prepStatement.setTimestamp(4, new Timestamp(System.currentTimeMillis()));
          prepStatement.setString(5, clcId);
          prepStatement.addBatch();
          count++;
          if (count == 1000) {
            count = 0;
            prepStatement.executeBatch();
            closeConn(null, prepStatement, null);
            prepStatement = null;
          }
        }
        if (count != 0 && prepStatement != null) {
          prepStatement.executeBatch();
        }
        connection.commit();
      } catch (Exception e) {
        try {
          connection.rollback();
        } catch (Exception ex) {
        }
        throw e;
      } finally {
        try {
          connection.setAutoCommit(autoCommit);
        } catch (Exception exc) {
        }
        closeConn(null, prepStatement, null);
      }
    } catch (Exception exc) {
      throw new SiebelException("Unable to update Delivery States", exc);
    } finally {
      closeConn(connection, null, null);
    }
  }

  private boolean shutdowned = true;

  /**
   * @noinspection EmptyCatchBlock
   */
  public void shutdown() {
    if (pool != null) {
      try {
        pool.shutdown();
      } catch (SQLException e) {
      }
    }
    shutdowned = true;
  }

  private static void closeConn(Connection connection, PreparedStatement preparedStatement,
                                java.sql.ResultSet resultSet) {

    if (resultSet != null) {
      try {
        resultSet.close();
      } catch (SQLException exc) {
        logger.error("Unable to close resultSet", exc);
      }
    }
    if (preparedStatement != null) {
      try {
        preparedStatement.close();
      } catch (SQLException exc) {
        logger.error("Unable to close preparedStatement", exc);
      }
    }
    if (connection != null) {
      try {
        connection.close();
      } catch (SQLException exc) {
        logger.error("Unable to close connection", exc);
      }
    }
  }

}
