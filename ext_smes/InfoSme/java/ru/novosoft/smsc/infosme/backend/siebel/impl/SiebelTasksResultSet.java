package ru.novosoft.smsc.infosme.backend.siebel.impl;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.infosme.backend.siebel.ResultSet;
import ru.novosoft.smsc.infosme.backend.siebel.SiebelException;
import ru.novosoft.smsc.infosme.backend.siebel.SiebelTask;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Date;
import java.util.Properties;

/**
 * author: alkhal
 */
public class SiebelTasksResultSet implements ResultSet {

  private static final Logger logger = Logger.getLogger(SiebelMessagesResultSet.class);
  private java.sql.ResultSet sqlResult;
  private Connection connection;
  private PreparedStatement prepStatement;
  private final Properties sql;

  public SiebelTasksResultSet(java.sql.ResultSet resultSet, Connection connection, PreparedStatement prepStatement, Properties sql) {
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
    SiebelTask siebelTask = new SiebelTask();
    try {
      siebelTask.setLastUpdate(new Date(sqlResult.getTimestamp(sql.getProperty("task.last.upd")).getTime()));
      siebelTask.setBeep(sqlResult.getString(sql.getProperty("task.beep")).equals("Y"));
      siebelTask.setCampaignId(sqlResult.getString(sql.getProperty("task.campaign.id")));
      siebelTask.setCreated(new Date(sqlResult.getTimestamp(sql.getProperty("task.created")).getTime()));
      siebelTask.setStatus(SiebelTask.Status.valueOf(
          sqlResult.getString(sql.getProperty("task.ctrl.status"))));
      siebelTask.setExpPeriod(new Integer(sqlResult.getInt(sql.getProperty("task.exp.period"))));
      siebelTask.setFlash(sqlResult.getString(sql.getProperty("task.flash")).equals("Y"));
      siebelTask.setPriority(sqlResult.getInt(sql.getProperty("task.priority")));
      siebelTask.setSave(sqlResult.getString(sql.getProperty("task.save")).equals("Y"));
      siebelTask.setWaveId(sqlResult.getString(sql.getProperty("task.wave.id")));
    } catch (Throwable e) {
      throw new SiebelException("Unable to get SiebelTask from the dataBase ", e);
    }
    return siebelTask;
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