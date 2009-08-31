package ru.novosoft.smsc.infosme.backend.siebel.impl;

import org.apache.log4j.Logger;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Properties;

import ru.novosoft.smsc.infosme.backend.siebel.ResultSet;
import ru.novosoft.smsc.infosme.backend.siebel.SmsMail;
import ru.novosoft.smsc.infosme.backend.siebel.IntegrationDataSourceException;

/**
 * author: alkhal
 */
public class SmsMailResultSet implements ResultSet{

  private static final Logger logger = Logger.getLogger(SmsMailResultSet.class);
  private java.sql.ResultSet sqlResult;
  private Connection connection;
  private PreparedStatement prepStatement;
  private final Properties sql;

  public SmsMailResultSet(java.sql.ResultSet resultSet, Connection connection, PreparedStatement prepStatement, Properties sql){
    this.connection = connection;
    this.sqlResult = resultSet;
    this.prepStatement = prepStatement;
    this.sql = sql;
  }

  public boolean next() throws IntegrationDataSourceException {
    try {
      return sqlResult.next();
    } catch (SQLException e) {
      throw new IntegrationDataSourceException("Unable to move resultset cursor", e);
    }
  }

  public Object get() throws IntegrationDataSourceException {
    SmsMail smsMail = new SmsMail();
    try {
      smsMail.setClcId(sqlResult.getString(sql.getProperty("sms.mail.clc.id")));
      smsMail.setCreated(new java.sql.Date(sqlResult.getTimestamp(sql.getProperty("sms.mail.created")).getTime()));
      smsMail.setLastUpd(new java.sql.Date(sqlResult.getTimestamp(sql.getProperty("sms.mail.last.upd")).getTime()));
      smsMail.setMessage(sqlResult.getString(sql.getProperty("sms.mail.message")));
      String ms = sqlResult.getString(sql.getProperty("sms.mail.message.state"));
      if(ms != null) {
        smsMail.setMessageState(SmsMail.MessageState.valueOf(ms));
      }
      smsMail.setMsisdn(sqlResult.getString(sql.getProperty("sms.mail.msisdn")));
      smsMail.setSmscCode(sqlResult.getString(sql.getProperty("sms.mail.smsc.stat.code")));
      smsMail.setSmscValue(sqlResult.getString(sql.getProperty("sms.mail.smsc.stat.val")));
      smsMail.setWaveId(sqlResult.getString(sql.getProperty("sms.mail.wave.id")));
    } catch (Throwable e) {
      throw new IntegrationDataSourceException("Unable to get SmsMail from the dataBase ", e);
    }
    return smsMail;
  }

  public void close() {
    try {
      if (sqlResult != null)
        sqlResult.close();
    } catch (SQLException e) {
      logger.error("Unable to close sql resultset ", e);
    }
    try {
      if (prepStatement != null)
        prepStatement.close();
    } catch (SQLException e) {
      logger.error("Unable to close sql resultset ", e);
    }
    try {
      if (connection != null)
        connection.close();
    } catch (SQLException e) {
      logger.error("Unable to close connection", e);
    }
  }

}
