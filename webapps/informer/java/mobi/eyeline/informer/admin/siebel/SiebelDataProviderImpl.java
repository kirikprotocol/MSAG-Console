package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import org.apache.log4j.Logger;
import ru.sibinco.util.conpool.ConnectionPool;

import java.io.IOException;
import java.io.InputStream;
import java.sql.*;
import java.util.Date;
import java.util.Map;
import java.util.Properties;

/**
 * @author Aleksandr Khalitov
 */
class SiebelDataProviderImpl implements SiebelDataProvider {

  private static final Logger logger = Logger.getLogger("SIEBEL");

  private static final int FETCH_SIZE = 1000;

  private ConnectionPool pool;

  private Properties sql;

  public SiebelDataProviderImpl() throws AdminException{
    try {
      InputStream is = this.getClass().getResourceAsStream("db.properties");
      sql = new Properties();
      try {
        sql.load(is);
      } finally {
        try {
          if (is != null)
            is.close();
        } catch (IOException ignored) {}
      }
    } catch (IOException e) {
      logger.error(e, e);
      throw new SiebelException("internal_error");
    }

  }
  /**
   * @noinspection EmptyCatchBlock
   */
  public void connect(Properties props) throws AdminException {
    if (pool != null) {
      try {
        pool.shutdown();
      } catch (SQLException e) {
      }
    }
    try {
      setDriver(props);
      pool = new ConnectionPool(props);
      shutdowned = false;
    } catch (SQLException e) {
      logger.error(e, e);
      throw new SiebelException("internal_error");
    }
  }

  private void setDriver(Properties props) {
    String dbType = props.getProperty("jdbc.pool.type");
    if(dbType.equals("mysql")) {
      props.setProperty("jdbc.driver", "com.mysql.jdbc.Driver");
    }else if(dbType.equals("oracle")) {
      props.setProperty("jdbc.driver", "oracle.jdbc.driver.OracleDriver");
    }else {
      throw new IllegalArgumentException("DB type is illegal: "+dbType);
    }
  }


  public void check(Properties props) throws AdminException {
    setDriver(props);
    try{
      Class.forName(props.getProperty("jdbc.driver"));
      Connection con = null;
      try{
        con = DriverManager.getConnection(props.getProperty("jdbc.source"),
            props.getProperty("jdbc.user"), props.getProperty("jdbc.password"));

        PreparedStatement prepStatement = null;
        try{
          prepStatement = con.prepareStatement(getSql("task.list.update"));
          prepStatement.setFetchSize(FETCH_SIZE);
          prepStatement.setTimestamp(1, new Timestamp(0));
          java.sql.ResultSet sqlResult = null;
          try{
            sqlResult = prepStatement.executeQuery();
            sqlResult.next();
          }finally {
            if(sqlResult != null) {
              try{
                sqlResult.close();
              }catch (SQLException ignored){}
            }
          }
        }finally {
          if(prepStatement != null) {
            try{
              prepStatement.close();
            }catch (SQLException ignored){}
          }
        }

      }finally {
        if(con != null) {
          try{
            con.close();
          }catch (SQLException ignored){}
        }
      }
    }catch (SQLException e){
      logger.error(e,e);
      throw new SiebelException("internal_error");
    } catch (ClassNotFoundException e) {
      logger.error(e,e);
      throw new SiebelException("internal_error");
    }
  }

  public boolean isShutdowned() {
    return shutdowned;
  }

  private String getSql(java.lang.String string) {
    return sql.getProperty(string);
  }

  public SiebelMessage getMessage(String clcId) throws AdminException {
    if (clcId == null) {
      throw new SiebelException("internal_error");
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
    } catch (Exception exc) {
      logger.error("Unable to get SiebelMessage from the dataBase with clcId: " + clcId, exc);
      throw new SiebelException("unable_get_data");

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return siebelMessage;
  }


  public ResultSet<SiebelMessage> getMessages(String waveId) throws AdminException {
    if (waveId == null) {
      throw new SiebelException("internal_error");
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
        logger.debug("Succesful get list of messages for wave="+waveId);
      }
    } catch (SQLException exc) {
      logger.error("Unable to get list of messages from the dataBase", exc);
      closeConn(connection, prepStatement, sqlResult);
      throw new SiebelException("unable_get_data");
    }
    return new SiebelMessagesResultSet(sqlResult, connection, prepStatement, sql);
  }

  public boolean containsUnfinished(String waveId) throws AdminException {
    if (waveId == null) {
      throw new SiebelException("internal_error");
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
    } catch (SQLException exc) {
      logger.error("Unable to get list of messages from the dataBase", exc);
      throw new SiebelException("unable_get_data");
    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
  }

  public SiebelDelivery getDelivery(String waveId) throws AdminException {
    if (waveId == null) {
      throw new SiebelException("internal_error");
    }
    SiebelDelivery siebelTask = null;
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
        siebelTask = new SiebelDelivery();
        siebelTask = new SiebelDelivery();
        siebelTask.setLastUpdate(new java.util.Date(sqlResult.getTimestamp(getSql("task.last.upd")).getTime()));
        siebelTask.setBeep(sqlResult.getString(getSql("task.beep")).equals("Y"));
        siebelTask.setCampaignId(sqlResult.getString(getSql("task.campaign.id")));
        siebelTask.setCreated(new java.util.Date(sqlResult.getTimestamp(getSql("task.created")).getTime()));
        siebelTask.setStatus(SiebelDelivery.Status.valueOf(
            sqlResult.getString(getSql("task.ctrl.status"))));
        siebelTask.setExpPeriod(sqlResult.getInt(getSql("task.exp.period")));
        siebelTask.setFlash(sqlResult.getString(getSql("task.flash")).equals("Y"));
        siebelTask.setPriority(sqlResult.getInt(getSql("task.priority")));
        siebelTask.setSave(sqlResult.getString(getSql("task.save")).equals("Y"));
        siebelTask.setWaveId(waveId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get siebelTask " + siebelTask);
      }
    } catch (SQLException exc) {
      logger.error("Unable to get SiebelDelivery from the dataBase with waveId: " + waveId, exc);
      throw new SiebelException("unable_get_data");

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return siebelTask;
  }

  public ResultSet<SiebelDelivery> getDeliveriesToUpdate(Date from) throws AdminException {

    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;

    try {
      connection = pool.getConnection();

      prepStatement = connection.prepareStatement(getSql("task.list.update"));
      prepStatement.setFetchSize(FETCH_SIZE);
      prepStatement.setTimestamp(1, new Timestamp(from.getTime()));

      sqlResult = prepStatement.executeQuery();

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get list of task from date="+from);
      }
    } catch (SQLException exc) {
      logger.error("Unable to get list of tasks from the dataBase", exc);
      closeConn(connection, prepStatement, sqlResult);
      throw new SiebelException("unable_get_data");
    }
    return new SiebelDeliveriesResultSet(sqlResult, connection, prepStatement, sql);
  }

  public ResultSet<SiebelDelivery> getDeliveries() throws AdminException {
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
    } catch (SQLException exc) {
      logger.error("Unable to get list of tasks from the dataBase", exc);
      closeConn(connection, prepStatement, sqlResult);
      throw new SiebelException("unable_get_data");
    }
    return new SiebelDeliveriesResultSet(sqlResult, connection, prepStatement, sql);
  }

  public void setDeliveryStatus(String waveId, SiebelDelivery.Status status) throws AdminException {
    if (status == null || waveId == null) {
      throw new SiebelException("internal_error");
    }

    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnection();
      prepStatement = connection.prepareStatement(getSql("task.set.status"));
      prepStatement.setString(1, status.toString());
      prepStatement.setString(2, waveId);
      int rowsUpdated = prepStatement.executeUpdate();

      if (rowsUpdated == 0) {
        throw new SiebelException("delivery_not_found", waveId);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful setting ctrl_status=" + status + " for waveId=" + waveId);
      }
    } catch (SQLException exc) {
      logger.error("Unable to set ctrl_status=" + status + " for waveId=" + waveId);
      throw new SiebelException("unable_get_data");
    } finally {
      closeConn(connection, prepStatement, null);
    }
  }



  @SuppressWarnings({"EmptyCatchBlock"})
  public void setDeliveryStatuses(Map<String, SiebelDelivery.Status> statuses) throws AdminException {
    if (statuses == null || statuses.isEmpty()) {
      return;
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnection();
      boolean autoCommit = connection.getAutoCommit();
      connection.setAutoCommit(false);
      try {
        int count = 0;
        for (Map.Entry<String, SiebelDelivery.Status> stringDeliveryStateEntry : statuses.entrySet()) {
          if (prepStatement == null) {
            prepStatement = connection.prepareStatement(getSql("task.set.status"));
          }
          String waveId = stringDeliveryStateEntry.getKey();
          SiebelDelivery.Status deliverySt = stringDeliveryStateEntry.getValue();
          prepStatement.setString(1, deliverySt.toString());
          prepStatement.setString(2, waveId);
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
      logger.error("Can't set deliveries statuses", exc);
      throw new SiebelException("unable_get_data");
    } finally {
      closeConn(connection, null, null);
    }
  }

  public SiebelDelivery.Status getDeliveryStatus(String waveId) throws AdminException {
    if (waveId == null) {
      throw new SiebelException("internal_error");
    }
    Connection connection = null;
    PreparedStatement prepStatement = null;
    java.sql.ResultSet sqlResult = null;
    SiebelDelivery.Status status = null;

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
          status = SiebelDelivery.Status.valueOf(st);
        }
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Succesful get delivery's status for: " + waveId);
      }
    } catch (SQLException exc) {
      logger.error("Unable to get SmsMailParam's status with waveId: " + waveId, exc);
      throw new SiebelException("unable_get_data");

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return status;
  }

  public SiebelMessage.State getMessageState(String clcId) throws AdminException {
    if (clcId == null) {
      throw new SiebelException("internal_error");
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
    } catch (SQLException exc) {
      logger.error("Unable to get SiebelMessage state with clcId: " + clcId, exc);
      throw new SiebelException("unable_get_data");

    } finally {
      closeConn(connection, prepStatement, sqlResult);
    }
    return state;
  }


  @SuppressWarnings({"EmptyCatchBlock"})
  public void setMessageStates(Map<String, SiebelMessage.DeliveryState> deliveryStates) throws AdminException {
    if (deliveryStates == null || deliveryStates.isEmpty()) {
      return;
    }

    Connection connection = null;
    PreparedStatement prepStatement = null;

    try {
      connection = pool.getConnection();
      boolean autoCommit = connection.getAutoCommit();
      connection.setAutoCommit(false);
      try {
        int count = 0;
        for (Map.Entry<String, SiebelMessage.DeliveryState> stringDeliveryStateEntry : deliveryStates.entrySet()) {
          if (prepStatement == null) {
            prepStatement = connection.prepareStatement(getSql("message.update.delivered"));
          }
          String clcId = stringDeliveryStateEntry.getKey();
          SiebelMessage.DeliveryState deliverySt = stringDeliveryStateEntry.getValue();
          prepStatement.setString(1, deliverySt.getSmppCode());
          prepStatement.setString(2, deliverySt.getSmppCodeDescription());
          prepStatement.setString(3, deliverySt.getState().toString());
          prepStatement.setString(4, clcId);
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
      logger.error("Unable to set messages statuses", exc);
      throw new SiebelException("unable_get_data");
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
//
//  public static void main(String args[]) throws AdminException {
//    SiebelDataProviderImpl d = new SiebelDataProviderImpl();
//    Properties props = new Properties();
//    props.setProperty("jdbc.source", "jdbc:oracle:thin:@10.0.172.197:1544:SIBDB");
//    props.setProperty("jdbc.driver", "oracle.jdbc.driver.OracleDriver");
//    props.setProperty("jdbc.user", "SMS_SENDER");
//    props.setProperty("jdbc.pass", "dsldk2lncflknS");
//    System.out.println("Connecting...");
//    d.connect(props);
//    System.out.println("Connected. Fetching tasks...");
//    long start = System.currentTimeMillis();
//      SiebelMessage m = d.getMessage("1-3964199480");
//    System.out.println((System.currentTimeMillis() - start) /  1000);
//    d.shutdown();
//  }

}
