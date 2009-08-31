package ru.novosoft.smsc.infosme.backend.siebel.impl;

import ru.novosoft.smsc.infosme.backend.siebel.ResultSet;

import java.io.InputStream;
import java.io.IOException;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.Date;
import java.util.Properties;

import org.apache.log4j.Logger;
import ru.sibinco.util.DBConnectionManager;
import ru.novosoft.smsc.infosme.backend.siebel.*;

/**
 * author: alkhal
 */
public class IntegrationDataSourceImpl implements IntegrationDataSource {

  private static final Logger logger = Logger.getLogger(IntegrationDataSourceImpl.class);

  private final DBConnectionManager pool;
  private final Properties sql;
  
  private static final String POOL_NAME = "default";

  public IntegrationDataSourceImpl() throws IntegrationDataSourceException {
    try{
      pool = DBConnectionManager.getInstance();
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

  public SmsMail getSmsMail(String clcId) throws IntegrationDataSourceException {
    if (clcId == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    SmsMail smsMail = null;
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnectionFromPool(POOL_NAME);

      prepStatement = connection.prepareStatement(getSql("sms.mail.get"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, clcId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        smsMail = new SmsMail();
        smsMail.setClcId(clcId);
        smsMail.setCreated(new Date(sqlResult.getTimestamp(sql.getProperty("sms.mail.created")).getTime()));
        smsMail.setLastUpd(new Date(sqlResult.getTimestamp(sql.getProperty("sms.mail.last.upd")).getTime()));
        smsMail.setMessage(sqlResult.getString(sql.getProperty("sms.mail.message")));
        String ms = sqlResult.getString(sql.getProperty("sms.mail.message.state"));
        if(ms != null) {
          smsMail.setMessageState(SmsMail.MessageState.valueOf(ms));
        }
        smsMail.setMsisdn(sqlResult.getString(sql.getProperty("sms.mail.msisdn")));
        smsMail.setSmscCode(sqlResult.getString(sql.getProperty("sms.mail.smsc.stat.code")));
        smsMail.setSmscValue(sqlResult.getString(sql.getProperty("sms.mail.smsc.stat.val")));
        smsMail.setWaveId(sqlResult.getString(sql.getProperty("sms.mail.wave.id")));
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get smsMail " + smsMail);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SmsMail from the dataBase with clcId: " + clcId, exc);
      throw new IntegrationDataSourceException("Unable to get SmsMail from the dataBase with clcId: " + clcId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return smsMail;
  }


  public ResultSet listSmsMail(String waveId) throws IntegrationDataSourceException {
    if (waveId == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnectionFromPool(POOL_NAME);

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

  public SmsMailParams getSmsMailParams(String waveId) throws IntegrationDataSourceException {
    if (waveId == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    SmsMailParams smsMailParams = null;
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnectionFromPool(POOL_NAME);

      prepStatement = connection.prepareStatement(getSql("sms.mail.params.get"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, waveId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        smsMailParams = new SmsMailParams();
        smsMailParams = new SmsMailParams();
        smsMailParams.setLastUpdate(new java.util.Date(sqlResult.getTimestamp(sql.getProperty("sms.mail.params.last.upd")).getTime()));
        smsMailParams.setBeep(sqlResult.getString(sql.getProperty("sms.mail.params.beep")).equals("Y"));
        smsMailParams.setCampaignId(sqlResult.getString(sql.getProperty("sms.mail.params.campaign.id")));
        smsMailParams.setCreated(new java.util.Date(sqlResult.getTimestamp(sql.getProperty("sms.mail.params.created")).getTime()));
        smsMailParams.setCtrlStatus(SmsMailParams.CtrlStatus.valueOf(
            sqlResult.getString(sql.getProperty("sms.mail.params.ctrl.status"))));
        smsMailParams.setExpPeriod(sqlResult.getInt(sql.getProperty("sms.mail.params.exp.period")));
        smsMailParams.setFlash(sqlResult.getString(sql.getProperty("sms.mail.params.flash")).equals("Y"));
        smsMailParams.setPriority(sqlResult.getInt(sql.getProperty("sms.mail.params.priority")));
        smsMailParams.setSave(sqlResult.getString(sql.getProperty("sms.mail.params.save")).equals("Y"));
        smsMailParams.setWaveId(waveId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get smsMailParams " + smsMailParams);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SmsMailParams from the dataBase with waveId: " + waveId, exc);
      throw new IntegrationDataSourceException("Unable to get SmsMailParams from the dataBase with waveId: " + waveId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return smsMailParams;
  }

  public ResultSet listSmsMailParams(Date fromUpdate) throws IntegrationDataSourceException {
    if (fromUpdate == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnectionFromPool(POOL_NAME);

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

  public ResultSet listSmsMailParams() throws IntegrationDataSourceException {
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnectionFromPool(POOL_NAME);

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

  public void setCtrlStatus(String waveId, SmsMailParams.CtrlStatus status) throws IntegrationDataSourceException {
    if (status == null || waveId == null) {
      throw new IntegrationDataSourceException("Some arguments are null");
    }

    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnectionFromPool(POOL_NAME);
      prepStatement = connection.prepareStatement(getSql("sms.mail.params.set.status"));
      prepStatement.setString(1, status.toString());
      prepStatement.setString(2, waveId);
      int rowsUpdated = prepStatement.executeUpdate();

      if(rowsUpdated == 0) {
        throw new IntegrationDataSourceException("SmsMailParams not found for waveId="+waveId);
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

  public SmsMailParams.CtrlStatus getCtrlStatus(String waveId) throws IntegrationDataSourceException {
    if (waveId == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;
    SmsMailParams.CtrlStatus status = null;

    try {
      connection = pool.getConnectionFromPool(POOL_NAME);

      prepStatement = connection.prepareStatement(getSql("sms.mail.params.get.status"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, waveId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        String st =
            sqlResult.getString(sql.getProperty("sms.mail.params.ctrl.status"));
        if(st != null) {
          status = SmsMailParams.CtrlStatus.valueOf(st);
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

  public void setSmsMailState(String clcId, SmsMail.MessageState state) throws IntegrationDataSourceException {
    if (state == null || clcId== null) {
      throw new IntegrationDataSourceException("Some arguments are null");
    }

    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnectionFromPool(POOL_NAME);
      prepStatement = connection.prepareStatement(getSql("sms.mail.set.status"));
      prepStatement.setString(1, state.toString());
      prepStatement.setString(2, clcId);
      int rowsUpdated = prepStatement.executeUpdate();

      if(rowsUpdated == 0) {
        throw new IntegrationDataSourceException("SmsMail not found for clcId="+clcId);
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

  public SmsMail.MessageState getSmsMailState(String clcId) throws IntegrationDataSourceException {
    if (clcId == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;
    SmsMail.MessageState state = null;

    try {
      connection = pool.getConnectionFromPool(POOL_NAME);

      prepStatement = connection.prepareStatement(getSql("sms.mail.get.status"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, clcId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        String st =
            sqlResult.getString(sql.getProperty("sms.mail.message.state"));
        if(st != null) {
          state = SmsMail.MessageState.valueOf(st);
        }
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get SmsMailParam's state " + state);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SmsMail state with clcId: " + clcId, exc);
      throw new IntegrationDataSourceException("Unable to get SmsMail state with clcId: " + clcId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return state;
  }

  public void setSmsMailSmscState(String clcId, String code, String value) throws IntegrationDataSourceException {
    if (clcId == null || code== null) {
      throw new IntegrationDataSourceException("Some arguments are null");
    }

    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnectionFromPool(POOL_NAME);
      prepStatement = connection.prepareStatement(getSql("sms.mail.set.status.smsc"));
      prepStatement.setString(1, code);
      prepStatement.setString(2, value);
      prepStatement.setString(3, clcId);
      int rowsUpdated = prepStatement.executeUpdate();

      if(rowsUpdated == 0) {
        throw new IntegrationDataSourceException("SmsMail not found for clcId="+clcId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful setting smscState='" + code +" "+ value +"' for clcId="+clcId);
      }
    } catch (SQLException exc) {
      logger.error("Unable to set smscState=" + code +" "+ value +"' for clcId="+clcId);
      throw new IntegrationDataSourceException("Unable to set smscState=" + code +" "+ value +"' for clcId="+clcId);
    } finally {
      closeConn(connection, prepStatement, null);
    }
  }

  public SmsMailSmscState getSmsMailSmscState(String clcId) throws IntegrationDataSourceException {
    if (clcId == null) {
      throw new IntegrationDataSourceException("Argument is null");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;
    SmsMailSmscState state = null;

    try {
      connection = pool.getConnectionFromPool(POOL_NAME);

      prepStatement = connection.prepareStatement(getSql("sms.mail.get.status.smsc"), java.sql.ResultSet.CONCUR_READ_ONLY);
      prepStatement.setFetchSize(Integer.MIN_VALUE);
      prepStatement.setString(1, clcId);

      sqlResult = prepStatement.executeQuery();

      if (sqlResult.next()) {
        state = new SmsMailSmscState(clcId);
        state.setCode(
            sqlResult.getString(sql.getProperty("sms.mail.smsc.stat.code"))
        );
        state.setValue(
            sqlResult.getString(sql.getProperty("sms.mail.smsc.stat.val"))
        );
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get SmsMail smsc state " + state);
      }
    } catch (Throwable exc) {
      logger.error("Unable to get SmsMail smsc state for clcId: " + clcId, exc);
      throw new IntegrationDataSourceException("Unable to get SmsMail smsc state for clcId: " + clcId, exc);

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return state;
  }

  private static void closeConn(Connection connection, PreparedStatement preparedStatement, java.sql.ResultSet resultSet) {
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
