package ru.novosoft.smsc.infosme.backend.siebel.impl;

import ru.novosoft.smsc.infosme.backend.siebel.ResultSet;

import java.io.InputStream;
import java.io.IOException;
import java.io.FileInputStream;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.*;

import org.apache.log4j.Logger;
import ru.sibinco.util.conpool.ConnectionPool;
import ru.novosoft.smsc.infosme.backend.siebel.*;

/**
 * author: alkhal
 */
public class SiebelDataProviderImpl implements SiebelDataProvider {

  private static final Logger logger = Logger.getLogger(SiebelDataProviderImpl.class);

  private final ConnectionPool pool;
  private final Properties sql;


  public SiebelDataProviderImpl(Properties props) throws SiebelException {
    try{
      pool = new ConnectionPool(props);
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
    }catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  private String getSql(java.lang.String string){
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

      prepStatement = connection.prepareStatement(getSql("message.get"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, clcId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        siebelMessage = new SiebelMessage();
        siebelMessage.setClcId(clcId);
        siebelMessage.setCreated(new Date(sqlResult.getTimestamp(sql.getProperty("message.created")).getTime()));
        siebelMessage.setLastUpd(new Date(sqlResult.getTimestamp(sql.getProperty("message.last.upd")).getTime()));
        siebelMessage.setMessage(sqlResult.getString(sql.getProperty("message.message")));
        String ms = sqlResult.getString(sql.getProperty("message.message.state"));
        if(ms != null) {
          siebelMessage.setMessageState(SiebelMessage.State.valueOf(ms));
        }
        siebelMessage.setMsisdn(sqlResult.getString(sql.getProperty("message.msisdn")));
        siebelMessage.setSmscCode(sqlResult.getString(sql.getProperty("message.smsc.stat.code")));
        siebelMessage.setSmscValue(sqlResult.getString(sql.getProperty("message.smsc.stat.val")));
        siebelMessage.setWaveId(sqlResult.getString(sql.getProperty("message.wave.id")));
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

      prepStatement = connection.prepareStatement(getSql("message.list"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
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

      prepStatement = connection.prepareStatement(getSql("task.get"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, waveId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        siebelTask = new SiebelTask();
        siebelTask = new SiebelTask();
        siebelTask.setLastUpdate(new java.util.Date(sqlResult.getTimestamp(sql.getProperty("task.last.upd")).getTime()));
        siebelTask.setBeep(sqlResult.getString(sql.getProperty("task.beep")).equals("Y"));
        siebelTask.setCampaignId(sqlResult.getString(sql.getProperty("task.campaign.id")));
        siebelTask.setCreated(new java.util.Date(sqlResult.getTimestamp(sql.getProperty("task.created")).getTime()));
        siebelTask.setStatus(SiebelTask.Status.valueOf(
            sqlResult.getString(sql.getProperty("task.ctrl.status"))));
        siebelTask.setExpPeriod(new Integer(sqlResult.getInt(sql.getProperty("task.exp.period"))));
        siebelTask.setFlash(sqlResult.getString(sql.getProperty("task.flash")).equals("Y"));
        siebelTask.setPriority(sqlResult.getInt(sql.getProperty("task.priority")));
        siebelTask.setSave(sqlResult.getString(sql.getProperty("task.save")).equals("Y"));
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

  public ResultSet getTasks(Date fromUpdate, Date tillUpdate) throws SiebelException {
    if (fromUpdate == null) {
      throw new SiebelException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("task.list.update"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setTimestamp(1, new Timestamp(fromUpdate.getTime()));
      prepStatement.setTimestamp(2, new Timestamp(tillUpdate.getTime()));

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

      prepStatement = connection.prepareStatement(getSql("task.list"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);

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

      if(rowsUpdated == 0) {
        throw new SiebelException("SiebelTask not found for waveId="+waveId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful setting ctrl_status=" + status+" for waveId="+waveId);
      }
    } catch (SQLException exc) {
      logger.error("Unable to set ctrl_status=" + status+" for waveId="+waveId);
      throw new SiebelException("Unable to set ctrl_status=" + status+" for waveId="+waveId, exc);
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

      prepStatement = connection.prepareStatement(getSql("task.get.status"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, waveId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        String st =
            sqlResult.getString(sql.getProperty("task.ctrl.status"));
        if(st != null) {
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
    if (state == null || clcId== null) {
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

      if(rowsUpdated == 0) {
        throw new SiebelException("SiebelMessage not found for clcId="+clcId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful setting state=" + state+" for clcId="+clcId);
      }
    } catch (SQLException exc) {
      logger.error("Unable to set state=" + state+" for clcId="+clcId);
      throw new SiebelException("Unable to set cstates=" + state+" for clcId="+clcId, exc);
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

      prepStatement = connection.prepareStatement(getSql("message.get.status"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, clcId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        String st =
            sqlResult.getString(sql.getProperty("message.message.state"));
        if(st != null) {
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

  public void updateDeliveryStates(Map deliveryStates) throws SiebelException {
    if(deliveryStates == null) {
      throw new SiebelException("Some arguments are null");
    }
    if(deliveryStates.isEmpty()) {
      return;
    }

    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnection();
      boolean autoCommit = connection.getAutoCommit();
      connection.setAutoCommit(false);

      prepStatement = connection.prepareStatement(getSql("message.update.delivered"));
      Iterator i = deliveryStates.entrySet().iterator();
      int count = 0;
      while(i.hasNext()) {
        Map.Entry e = (Map.Entry)i.next();
        String clcId = (String)e.getKey();
        SiebelMessage.DeliveryState deliverySt = (SiebelMessage.DeliveryState)e.getValue();
        prepStatement.setString(1, deliverySt.getSmppCode());
        prepStatement.setString(2, deliverySt.getSmppCodeDescription());
        prepStatement.setString(3, deliverySt.getState().toString());
        prepStatement.setTimestamp(4, new Timestamp(System.currentTimeMillis()));
        prepStatement.setString(5, clcId);
        prepStatement.addBatch();
        count++;
        if(count == 1000) {
          count = 0;
          prepStatement.executeBatch();
        }
      }
      if(count != 0) {
        prepStatement.executeBatch();
      }
      connection.commit();
      connection.setAutoCommit(autoCommit );
    } catch (SQLException exc) {
      throw new SiebelException("Unable to update Delivery States=", exc);
    } finally {
      closeConn(connection, prepStatement, null);
    }

  }

  public void shutdown() {
    if(pool != null) {
      try {
        pool.shutdown();
      } catch (SQLException e) {}
    }
  }

  private static void closeConn(Connection connection, PreparedStatement preparedStatement,
                                java.sql.ResultSet resultSet) {
    try {
      if (connection != null) {
        connection.close();
      }
    } catch (SQLException exc) {
      logger.error("Unable to close connection", exc);
    }
    try {
      if (preparedStatement != null) {
        preparedStatement.close();
      }
    } catch (SQLException exc) {
      logger.error("Unable to close connection", exc);

    }
    try {
      if (resultSet != null) {
        resultSet.close();
      }
    } catch (SQLException exc) {
      logger.error("Unable to close connection", exc);
    }
  }

}
