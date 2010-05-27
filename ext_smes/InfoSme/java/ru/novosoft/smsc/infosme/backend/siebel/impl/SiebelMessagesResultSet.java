package ru.novosoft.smsc.infosme.backend.siebel.impl;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.infosme.backend.siebel.ResultSet;
import ru.novosoft.smsc.infosme.backend.siebel.SiebelException;
import ru.novosoft.smsc.infosme.backend.siebel.SiebelMessage;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Properties;

/**
 * author: alkhal
 */
public class SiebelMessagesResultSet implements ResultSet {

  private static final Logger logger = Logger.getLogger(SiebelMessagesResultSet.class);
  private java.sql.ResultSet sqlResult;
  private Connection connection;
  private PreparedStatement prepStatement;
  private final Properties sql;

  public SiebelMessagesResultSet(java.sql.ResultSet resultSet, Connection connection, PreparedStatement prepStatement, Properties sql) {
    this.connection = connection;
    this.sqlResult = resultSet;
    this.prepStatement = prepStatement;
    this.sql = sql;
  }

  public boolean next() throws SiebelException {
    try {
      return sqlResult.next();
    } catch (SQLException e) {
      throw new SiebelException("Unable to move resultset cursor", e);
    }
  }

  public Object get() throws SiebelException {
    SiebelMessage siebelMessage = new SiebelMessage();
    try {
      siebelMessage.setClcId(sqlResult.getString(sql.getProperty("message.clc.id")));
      siebelMessage.setCreated(new java.sql.Date(sqlResult.getTimestamp(sql.getProperty("message.created")).getTime()));
      siebelMessage.setLastUpd(new java.sql.Date(sqlResult.getTimestamp(sql.getProperty("message.last.upd")).getTime()));
      siebelMessage.setMessage(sqlResult.getString(sql.getProperty("message.message")));
      String ms = sqlResult.getString(sql.getProperty("message.message.state"));
      if (ms != null) {
        siebelMessage.setMessageState(SiebelMessage.State.valueOf(ms));
      }
      siebelMessage.setMsisdn(sqlResult.getString(sql.getProperty("message.msisdn")));
      siebelMessage.setSmscCode(sqlResult.getString(sql.getProperty("message.smsc.stat.code")));
      siebelMessage.setSmscValue(sqlResult.getString(sql.getProperty("message.smsc.stat.val")));
      siebelMessage.setWaveId(sqlResult.getString(sql.getProperty("message.wave.id")));
    } catch (Throwable e) {
      throw new SiebelException("Unable to get SiebelMessage from the dataBase ", e);
    }
    return siebelMessage;
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
