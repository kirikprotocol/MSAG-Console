package ru.novosoft.smsc.infosme.backend.siebel.impl;

import ru.novosoft.smsc.infosme.backend.siebel.ResultSet;

import java.io.InputStream;
import java.io.IOException;
import java.io.FileInputStream;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.Date;
import java.util.Properties;
import java.util.Map;

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


  public SiebelDataProviderImpl(String config) throws IntegrationDataSourceException {
    InputStream in = null;
    try{
      Properties jdbcProp = new Properties();
      try{
        in = new FileInputStream(config);
        jdbcProp.load(in);
      }finally {
        if(in != null) {
          try{
            in.close();
          }catch (IOException e) {}
        }
      }
      pool = new ConnectionPool(jdbcProp);
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
      throw new IntegrationDataSourceException(e);
    }
  }

  private String getSql(java.lang.String string) throws IntegrationDataSourceException {
    return sql.getProperty(string);
  }

  public SiebelMessage getMessage(String clcId) throws IntegrationDataSourceException {
    if (clcId == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    SiebelMessage siebelMessage = null;
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("sms.mail.get"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, clcId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        siebelMessage = new SiebelMessage();
        siebelMessage.setClcId(clcId);
        siebelMessage.setCreated(new Date(sqlResult.getTimestamp(sql.getProperty("sms.mail.created")).getTime()));
        siebelMessage.setLastUpd(new Date(sqlResult.getTimestamp(sql.getProperty("sms.mail.last.upd")).getTime()));
        siebelMessage.setMessage(sqlResult.getString(sql.getProperty("sms.mail.message")));
        String ms = sqlResult.getString(sql.getProperty("sms.mail.message.state"));
        if(ms != null) {
          siebelMessage.setMessageState(SiebelMessage.State.valueOf(ms));
        }
        siebelMessage.setMsisdn(sqlResult.getString(sql.getProperty("sms.mail.msisdn")));
        siebelMessage.setSmscCode(sqlResult.getString(sql.getProperty("sms.mail.smsc.stat.code")));
        siebelMessage.setSmscValue(sqlResult.getString(sql.getProperty("sms.mail.smsc.stat.val")));
        siebelMessage.setWaveId(sqlResult.getString(sql.getProperty("sms.mail.wave.id")));
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get siebelMessage " + siebelMessage);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SiebelMessage from the dataBase with clcId: " + clcId, exc);
      throw new IntegrationDataSourceException("Unable to get SiebelMessage from the dataBase with clcId: " + clcId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return siebelMessage;
  }


  public ResultSet getMessages(String waveId) throws IntegrationDataSourceException {
    if (waveId == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("sms.mail.list"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, waveId);

      sqlResult = prepStatement.executeQuery();

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get list of smsMails");
      }
    } catch (SQLException exc) {
      logger.error("Unable to get list of smsMails from the dataBase", exc);
      closeConn(connection, prepStatement, sqlResult);
      throw new IntegrationDataSourceException("Unable to get list of smsMails from the dataBase", exc);
    }
    return new SmsMailResultSet(sqlResult, connection, prepStatement, sql);
  }

  public SiebelTask getTask(String waveId) throws IntegrationDataSourceException {
    if (waveId == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    SiebelTask siebelTask = null;
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("sms.mail.params.get"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, waveId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        siebelTask = new SiebelTask();
        siebelTask = new SiebelTask();
        siebelTask.setLastUpdate(new java.util.Date(sqlResult.getTimestamp(sql.getProperty("sms.mail.params.last.upd")).getTime()));
        siebelTask.setBeep(sqlResult.getString(sql.getProperty("sms.mail.params.beep")).equals("Y"));
        siebelTask.setCampaignId(sqlResult.getString(sql.getProperty("sms.mail.params.campaign.id")));
        siebelTask.setCreated(new java.util.Date(sqlResult.getTimestamp(sql.getProperty("sms.mail.params.created")).getTime()));
        siebelTask.setCtrlStatus(SiebelTask.CtrlStatus.valueOf(
            sqlResult.getString(sql.getProperty("sms.mail.params.ctrl.status"))));
        siebelTask.setExpPeriod(new Integer(sqlResult.getInt(sql.getProperty("sms.mail.params.exp.period"))));
        siebelTask.setFlash(sqlResult.getString(sql.getProperty("sms.mail.params.flash")).equals("Y"));
        siebelTask.setPriority(sqlResult.getInt(sql.getProperty("sms.mail.params.priority")));
        siebelTask.setSave(sqlResult.getString(sql.getProperty("sms.mail.params.save")).equals("Y"));
        siebelTask.setWaveId(waveId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get siebelTask " + siebelTask);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SiebelTask from the dataBase with waveId: " + waveId, exc);
      throw new IntegrationDataSourceException("Unable to get SiebelTask from the dataBase with waveId: " + waveId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return siebelTask;
  }

  public ResultSet getTasks(Date fromUpdate) throws IntegrationDataSourceException {
    if (fromUpdate == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("sms.mail.params.list.from.update"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setTimestamp(1, new Timestamp(fromUpdate.getTime()));

      sqlResult = prepStatement.executeQuery();

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get list of smsMailParams");
      }
    } catch (SQLException exc) {
      logger.error("Unable to get list of smsMailParams from the dataBase", exc);
      closeConn(connection, prepStatement, sqlResult);
      throw new IntegrationDataSourceException("Unable to get list of smsMailParams from the dataBase", exc);
    }
    return new SmsMailParamsResultSet(sqlResult, connection, prepStatement, sql);
  }

  public ResultSet getTasks() throws IntegrationDataSourceException {
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("sms.mail.params.list"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);

      sqlResult = prepStatement.executeQuery();

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get list of smsMails");
      }
    } catch (SQLException exc) {
      logger.error("Unable to get list of smsMailParams from the dataBase", exc);
      closeConn(connection, prepStatement, sqlResult);
      throw new IntegrationDataSourceException("Unable to get list of smsMailParams from the dataBase", exc);
    }
    return new SmsMailParamsResultSet(sqlResult, connection, prepStatement, sql);
  }

  public void setTaskStatus(String waveId, SiebelTask.CtrlStatus status) throws IntegrationDataSourceException {
    if (status == null || waveId == null) {
      throw new IntegrationDataSourceException("Some arguments are null");
    }

    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnection();
      prepStatement = connection.prepareStatement(getSql("sms.mail.params.set.status"));
      prepStatement.setString(1, status.toString());
      prepStatement.setString(2, waveId);
      int rowsUpdated = prepStatement.executeUpdate();

      if(rowsUpdated == 0) {
        throw new IntegrationDataSourceException("SiebelTask not found for waveId="+waveId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful setting ctrl_status=" + status+" for waveId="+waveId);
      }
    } catch (SQLException exc) {
      logger.error("Unable to set ctrl_status=" + status+" for waveId="+waveId);
      throw new IntegrationDataSourceException("Unable to set ctrl_status=" + status+" for waveId="+waveId, exc);
    } finally {
      closeConn(connection, prepStatement, null);
    }
  }

  public SiebelTask.CtrlStatus getTaskStatus(String waveId) throws IntegrationDataSourceException {
    if (waveId == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;
    SiebelTask.CtrlStatus status = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("sms.mail.params.get.status"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, waveId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        String st =
            sqlResult.getString(sql.getProperty("sms.mail.params.ctrl.status"));
        if(st != null) {
          status = SiebelTask.CtrlStatus.valueOf(st);
        }
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get SmsMailParam's status " + status);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SmsMailParam's status with waveId: " + waveId, exc);
      throw new IntegrationDataSourceException("Unable to get SmsMailParam's status with waveId: " + waveId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return status;
  }

  public void setMessageState(String clcId, SiebelMessage.State state) throws IntegrationDataSourceException {
    if (state == null || clcId== null) {
      throw new IntegrationDataSourceException("Some arguments are null");
    }

    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnection();        
      prepStatement = connection.prepareStatement(getSql("sms.mail.set.status"));
      prepStatement.setString(1, state.toString());
      prepStatement.setString(2, clcId);
      int rowsUpdated = prepStatement.executeUpdate();

      if(rowsUpdated == 0) {
        throw new IntegrationDataSourceException("SiebelMessage not found for clcId="+clcId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful setting state=" + state+" for clcId="+clcId);
      }
    } catch (SQLException exc) {
      logger.error("Unable to set state=" + state+" for clcId="+clcId);
      throw new IntegrationDataSourceException("Unable to set cstates=" + state+" for clcId="+clcId, exc);
    } finally {
      closeConn(connection, prepStatement, null);
    }
  }

  public SiebelMessage.State getMessageState(String clcId) throws IntegrationDataSourceException {
    if (clcId == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;
    SiebelMessage.State state = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("sms.mail.get.status"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, clcId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        String st =
            sqlResult.getString(sql.getProperty("sms.mail.message.state"));
        if(st != null) {
          state = SiebelMessage.State.valueOf(st);
        }
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get SmsMailParam's state " + state);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SiebelMessage state with clcId: " + clcId, exc);
      throw new IntegrationDataSourceException("Unable to get SiebelMessage state with clcId: " + clcId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return state;
  }



  public void setMessageSmppState(String clcId, SiebelMessage.SmppState state) throws IntegrationDataSourceException {
    if (clcId == null || state== null) {
      throw new IntegrationDataSourceException("Some arguments are null");
    }

    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnection();
      prepStatement = connection.prepareStatement(getSql("sms.mail.set.status.smsc"));
      prepStatement.setString(1, state.getCode());
      prepStatement.setString(2, state.getValue());
      prepStatement.setString(3, clcId);
      int rowsUpdated = prepStatement.executeUpdate();

      if(rowsUpdated == 0) {
        throw new IntegrationDataSourceException("SiebelMessage not found for clcId="+clcId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful setting smscState='" + state.getCode() +" "+ state.getValue() +"' for clcId="+clcId);
      }
    } catch (SQLException exc) {
      logger.error("Unable to set smscState=" + state.getCode() +" "+ state.getValue() +"' for clcId="+clcId);
      throw new IntegrationDataSourceException("Unable to set smscState=" + state.getCode() +" "+ state.getValue() +"' for clcId="+clcId);
    } finally {
      closeConn(connection, prepStatement, null);
    }
  }

  public SiebelMessage.SmppState getMessageSmppState(String clcId) throws IntegrationDataSourceException {
    if (clcId == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;
    SiebelMessage.SmppState state = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("sms.mail.get.status.smsc"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, clcId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        state = new SiebelMessage.SmppState();
        state.setCode(
            sqlResult.getString(sql.getProperty("sms.mail.smsc.stat.code"))
        );
        state.setValue(
            sqlResult.getString(sql.getProperty("sms.mail.smsc.stat.val"))
        );
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get SiebelMessage smsc state " + state);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SiebelMessage smsc state for clcId: " + clcId, exc);
      throw new IntegrationDataSourceException("Unable to get SiebelMessage smsc state for clcId: " + clcId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return state;
  }

  public void updateDeliveryStates(Map deliveryStates) {
    //todo
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
