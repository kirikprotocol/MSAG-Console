package ru.novosoft.smsc.infosme.backend.siebel.impl;

import ru.novosoft.smsc.infosme.backend.siebel.ResultSet;
import ru.novosoft.smsc.infosme.backend.siebel.SmsMailParams;
import ru.novosoft.smsc.infosme.backend.siebel.IntegrationDataSourceException;
import org.apache.log4j.Logger;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Date;
import java.util.Properties;

/**
 * author: alkhal
 */
public class SmsMailParamsResultSet implements ResultSet {

  private static final Logger logger = Logger.getLogger(SmsMailResultSet.class);
  private java.sql.ResultSet sqlResult;
  private Connection connection;
  private PreparedStatement prepStatement;
  private final Properties sql;

  public SmsMailParamsResultSet(java.sql.ResultSet resultSet, Connection connection, PreparedStatement prepStatement, Properties sql){
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
    SmsMailParams smsMailParams = new SmsMailParams();
    try {
      smsMailParams.setLastUpdate(new Date(sqlResult.getTimestamp(sql.getProperty("sms.mail.params.last.upd")).getTime()));
      smsMailParams.setBeep(sqlResult.getString(sql.getProperty("sms.mail.params.beep")).equals("Y"));
      smsMailParams.setCampaignId(sqlResult.getString(sql.getProperty("sms.mail.params.campaign.id")));
      smsMailParams.setCreated(new Date(sqlResult.getTimestamp(sql.getProperty("sms.mail.params.created")).getTime()));
      smsMailParams.setCtrlStatus(SmsMailParams.CtrlStatus.valueOf(
          sqlResult.getString(sql.getProperty("sms.mail.params.ctrl.status"))));
      smsMailParams.setExpPeriod(sqlResult.getInt(sql.getProperty("sms.mail.params.exp.period")));
      smsMailParams.setFlash(sqlResult.getString(sql.getProperty("sms.mail.params.flash")).equals("Y"));
      smsMailParams.setPriority(sqlResult.getInt(sql.getProperty("sms.mail.params.priority")));
      smsMailParams.setSave(sqlResult.getString(sql.getProperty("sms.mail.params.save")).equals("Y"));
      smsMailParams.setWaveId(sqlResult.getString(sql.getProperty("sms.mail.params.wave.id")));
    } catch (Throwable e) {
      throw new IntegrationDataSourceException("Unable to get SmsMailParams from the dataBase ", e);
    }
    return smsMailParams;
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
