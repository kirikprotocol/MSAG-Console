package mobi.eyeline.informer.admin.siebel;

import org.apache.log4j.Logger;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Properties;

/**
 * @author Aleksandr Khalitov
 */
class SiebelMessagesResultSet implements ResultSet<SiebelMessage> {
  private static final Logger logger = Logger.getLogger("SIEBEL");
  private final java.sql.ResultSet sqlResult;
  private final Connection connection;
  private final PreparedStatement prepStatement;
  private final Properties sql;

  SiebelMessagesResultSet(java.sql.ResultSet resultSet, Connection connection, PreparedStatement prepStatement, Properties sql) {
    this.connection = connection;
    this.sqlResult = resultSet;
    this.prepStatement = prepStatement;
    this.sql = sql;
  }

  public boolean next() throws SiebelException {
    try {
      return sqlResult.next();
    } catch (SQLException e) {
      throw new SiebelException("unable_move_cursor");
    }
  }

  public SiebelMessage get() throws SiebelException {
    SiebelMessage siebelMessage = new SiebelMessage();
    try {
      siebelMessage.setClcId(sqlResult.getString(sql.getProperty("message.clc.id")));
      siebelMessage.setCreated(new java.sql.Date(sqlResult.getTimestamp(sql.getProperty("message.created")).getTime()));
      siebelMessage.setLastUpd(new java.sql.Date(sqlResult.getTimestamp(sql.getProperty("message.last.upd")).getTime()));
      siebelMessage.setMessage(sqlResult.getString(sql.getProperty("message.message")));
      String ms = sqlResult.getString(sql.getProperty("message.message.state"));
      if (ms != null) {
        siebelMessage.setMessageState(ms);
      }
      siebelMessage.setMsisdn(sqlResult.getString(sql.getProperty("message.msisdn")));
      siebelMessage.setSmscCode(sqlResult.getString(sql.getProperty("message.smsc.stat.code")));
      siebelMessage.setSmscValue(sqlResult.getString(sql.getProperty("message.smsc.stat.val")));
      siebelMessage.setWaveId(sqlResult.getString(sql.getProperty("message.wave.id")));
    } catch (SQLException e) {
      logger.error(e, e);
      throw new SiebelException("unable_get_data");
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
