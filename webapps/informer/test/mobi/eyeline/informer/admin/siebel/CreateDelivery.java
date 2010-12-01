package mobi.eyeline.informer.admin.siebel;

import org.junit.BeforeClass;
import org.junit.Test;
import ru.sibinco.util.conpool.ConnectionPool;

import java.io.IOException;
import java.io.InputStream;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Properties;

/**
 * @author Aleksandr Khalitov
 */
public class CreateDelivery {

  @BeforeClass
  public static void before() throws Exception{
    Properties ps = new Properties();
    InputStream is = null;
    try{
      is = CreateDelivery.class.getResourceAsStream("jdbc.properties");
      ps.load(is);
    }finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException ignored){}
      }
    }
    new ConnectionPool(ps);
  }


  @Test
  public void create() throws SQLException {
    CreateDelivery.createDelivery(1);
  }

  @Test
  public void remove() throws Exception {
    CreateDelivery.removeDelivery(1);
  }



  static void _createMessages(int wid, Connection connection) throws SQLException {
    int firstmsgid = -1;
    int msgid = wid-1000;
    PreparedStatement prepStatement = null;
    try {
      prepStatement = connection.prepareStatement("insert into SMS_MAIL (ROW_ID, WAVE_INT_ID, " +
          "CLC_INT_ID, CREATED, LAST_UPD, MSISDN, MESSAGE, MESSAGE_STATE, SMSC_STAT_CODE, SMSC_STAT_VAL) values (?,?,?,CURRENT_TIMESTAMP,CURRENT_TIMESTAMP,?,?,?,?,?)");
      int count = 0;

      System.out.println("Creation of SMS_MAIL table...");
      int wavesize = 1000;
      for (int i = 0; i < wavesize; i++) {
        prepStatement.setString(1, Integer.toString(msgid));
        prepStatement.setString(2, Integer.toString(wid));
        prepStatement.setString(3, Integer.toString(msgid));
        if (i%10 == 0) {
          prepStatement.setString(4, "invalid_msisdn");
        } else if (i%10 == 1) {
          prepStatement.setString(4, "+" + Long.toString(Long.parseLong("79520000000") + i));
        } else if (i%10 > 1 && i%10 <= 5) {
          prepStatement.setString(4, "+" + Long.toString(Long.parseLong("79160000000") + i));
        } else {
          prepStatement.setString(4, "+" + Long.toString(Long.parseLong("79130000000") + i));
        }
        prepStatement.setString(5, "TestTest");
        prepStatement.setNull(6, java.sql.Types.VARCHAR);
        prepStatement.setNull(7, java.sql.Types.VARCHAR);
        prepStatement.setNull(8, java.sql.Types.VARCHAR);
        prepStatement.addBatch();
        ++msgid;
        count++;
        if (count == 1000) {
          count = 0;
          prepStatement.executeBatch();
        }
      }
      if (count != 0) {
        prepStatement.executeBatch();
      }
    } finally {
      if (prepStatement != null) {
        prepStatement.close();
      }
    }
    System.out.println("Completed: " + (msgid - firstmsgid) + " messages");
    System.out.println();
  }

  static void _createDelivery(Connection connection, int wid) throws SQLException {

    PreparedStatement prepStatement = null;
    try {
      prepStatement = connection.prepareStatement("insert into SMS_MAIL_PARAMS (ROW_ID, WAVE_INT_ID, CAMPAIGN_INT_ID, " +
          "CREATED, LAST_UPD, PRIORITY, FLASH_FLG, SAVE_FLG, BEEP_FLG, EXP_PERIOD, CTRL_STATUS) values (?,?,?,CURRENT_TIMESTAMP,CURRENT_TIMESTAMP,?,?,?,?,?,?)");


      System.out.println("Creation of SMS_MAIL_PARAMS table...");
      prepStatement.setString(1, Integer.toString(wid));
      prepStatement.setString(2, Integer.toString(wid));
      prepStatement.setString(3, Integer.toString(wid));
      prepStatement.setString(4, Integer.toString(wid%100+1));
      prepStatement.setString(5,"Y");
      prepStatement.setString(6,"Y");
      prepStatement.setString(7,"N");
      prepStatement.setString(8, "12");
      prepStatement.setString(9, SiebelDelivery.Status.ENQUEUED.toString());

      prepStatement.execute();

    } finally {
      if (prepStatement != null) {
        prepStatement.close();
      }
    }
  }

  static void createDelivery(int wid) throws SQLException{
    Connection connection = null;

    try {
      connection = ConnectionPool.getPool("default").getConnection();
      connection.setAutoCommit(false);

      _createMessages(wid, connection);

      _createDelivery(connection, wid);

      connection.commit();
      System.out.println("Completed:  wave");

    } finally {
      if(connection != null) {
        connection.setAutoCommit(true);
        connection.close();
      }
    }
  }


  static void removeDelivery(int wid) throws SQLException{

    Connection connection = null;

    try {
      connection = ConnectionPool.getPool("default").getConnection();

      PreparedStatement prepStatement = null;
      try {
        prepStatement = connection.prepareStatement("DELETE FROM SMS_MAIL where WAVE_INT_ID=?");
        prepStatement.setString(1, Integer.toString(wid));
        prepStatement.executeUpdate();
      } finally {
        if (prepStatement != null) {
          prepStatement.close();
        }
      }

      prepStatement = null;
      try {
        prepStatement = connection.prepareStatement("DELETE FROM SMS_MAIL_PARAMS where WAVE_INT_ID=?");
        prepStatement.setString(1, Integer.toString(wid));
        prepStatement.executeUpdate();
      } finally {
        if (prepStatement != null) {
          prepStatement.close();
        }
      }
      System.out.println("Removing tables: Finished");
    } finally {
      try {
        if (connection != null) {
          connection.close();
        }
      } catch (Throwable ignored) {
      }
    }
  }
}
