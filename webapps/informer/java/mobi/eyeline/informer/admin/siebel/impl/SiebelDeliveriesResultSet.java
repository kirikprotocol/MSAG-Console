package mobi.eyeline.informer.admin.siebel.impl;

import mobi.eyeline.informer.admin.siebel.ResultSet;
import mobi.eyeline.informer.admin.siebel.SiebelDelivery;
import mobi.eyeline.informer.admin.siebel.SiebelException;
import org.apache.log4j.Logger;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Date;
import java.util.Properties;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelDeliveriesResultSet implements ResultSet<SiebelDelivery> {

  private static final Logger logger = Logger.getLogger(SiebelMessagesResultSet.class);
  private java.sql.ResultSet sqlResult;
  private Connection connection;
  private PreparedStatement prepStatement;
  private final Properties sql;

  public SiebelDeliveriesResultSet(java.sql.ResultSet resultSet, Connection connection, PreparedStatement prepStatement, Properties sql) {
    this.connection = connection;
    this.sqlResult = resultSet;
    this.prepStatement = prepStatement;
    this.sql = sql;
  }

  public boolean next() throws SiebelException {
    try {
      return sqlResult.next();
    } catch (SQLException e) {
      logger.error(e,e);
      throw new SiebelException("unable_move_cursor");
    }
  }

  public SiebelDelivery get() throws SiebelException {
    SiebelDelivery delivery = new SiebelDelivery();
    try {
      delivery.setLastUpdate(new Date(sqlResult.getTimestamp(sql.getProperty("task.last.upd")).getTime()));
      delivery.setBeep(sqlResult.getString(sql.getProperty("task.beep")).equals("Y"));
      delivery.setCampaignId(sqlResult.getString(sql.getProperty("task.campaign.id")));
      delivery.setCreated(new Date(sqlResult.getTimestamp(sql.getProperty("task.created")).getTime()));
      delivery.setStatus(SiebelDelivery.Status.valueOf(
          sqlResult.getString(sql.getProperty("task.ctrl.status"))));
      delivery.setExpPeriod(sqlResult.getInt(sql.getProperty("task.exp.period")));
      delivery.setFlash(sqlResult.getString(sql.getProperty("task.flash")).equals("Y"));
      delivery.setPriority(sqlResult.getInt(sql.getProperty("task.priority")));
      delivery.setSave(sqlResult.getString(sql.getProperty("task.save")).equals("Y"));
      delivery.setWaveId(sqlResult.getString(sql.getProperty("task.wave.id")));
    } catch (Throwable e) {
      logger.error(e,e);
      throw new SiebelException("unable_get_data");
    }
    return delivery;
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
      logger.error("Unable to close sql prep. statement ", e);
    }
    try {
      if (connection != null)
        connection.close();
    } catch (SQLException e) {
      logger.error("Unable to close connection", e);
    }
  }

}