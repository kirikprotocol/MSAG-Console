package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.siebel.impl.SiebelDeliveries;
import mobi.eyeline.informer.admin.siebel.impl.SiebelRegionManager;
import ru.sibinco.util.conpool.ConnectionPool;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Timestamp;

/**
 * @author Aleksandr Khalitov
 */
public class TestSiebelManager extends SiebelManager{

  public TestSiebelManager(SiebelDeliveries deliveries, SiebelRegionManager regionManager) throws AdminException {
    super(deliveries, regionManager);
  }

  void _createDelivery(int wid, Connection connection) throws SQLException {
    int firstmsgid = -1;
    int msgid = firstmsgid;
    PreparedStatement prepStatement = null;
    try {
      prepStatement = connection.prepareStatement("insert into SMS_MAIL (ROW_ID, WAVE_INT_ID, " +
          "CLC_INT_ID, CREATED, LAST_UPD, MSISDN, MESSAGE, MESSAGE_STATE, SMSC_STAT_CODE, SMSC_STAT_VAL) values (?,?,?,?,?,?,?,?,?,?)");
      int count = 0;

      System.out.println("Creation of SMS_MAIL table...");
      int wavesize = 1000;
      for (int i = 0; i < wavesize; i++) {
        prepStatement.setString(1, Integer.toString(msgid));
        prepStatement.setString(2, Integer.toString(wid));
        prepStatement.setString(3, Integer.toString(msgid));
        prepStatement.setTimestamp(4, new Timestamp(System.currentTimeMillis()));
        prepStatement.setTimestamp(5, new Timestamp(System.currentTimeMillis()));
        if (i%10 == 0) {
          prepStatement.setString(6, "invalid_msisdn");
        } else if (i%10 == 1) {
          prepStatement.setString(6, "+" + Long.toString(Long.parseLong("79520000000") + i));
        } else if (i%10 > 1 && i%10 <= 5) {
          prepStatement.setString(6, "+" + Long.toString(Long.parseLong("79160000000") + i));
        } else {
          prepStatement.setString(6, "+" + Long.toString(Long.parseLong("79130000000") + i));
        }
        prepStatement.setString(7, "TestTest");
        prepStatement.setNull(8, java.sql.Types.VARCHAR);
        prepStatement.setNull(9, java.sql.Types.VARCHAR);
        prepStatement.setNull(10, java.sql.Types.VARCHAR);
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

  private void _createMessages(Connection connection, int wid) throws SQLException {

    PreparedStatement prepStatement = null;
    try {
      prepStatement = connection.prepareStatement("insert into SMS_MAIL_PARAMS (ROW_ID, WAVE_INT_ID, CAMPAIGN_INT_ID, " +
          "CREATED, LAST_UPD, PRIORITY, FLASH_FLG, SAVE_FLG, BEEP_FLG, EXP_PERIOD, CTRL_STATUS) values (?,?,?,?,?,?,?,?,?,?,?)");


      System.out.println("Creation of SMS_MAIL_PARAMS table...");
      prepStatement.setString(1, Integer.toString(wid));
      prepStatement.setString(2, Integer.toString(wid));
      prepStatement.setString(3, Integer.toString(wid));
      prepStatement.setTimestamp(4, new Timestamp(System.currentTimeMillis()));
      prepStatement.setTimestamp(5, new Timestamp(System.currentTimeMillis()));
      prepStatement.setString(6, Integer.toString(wid%100));
      prepStatement.setString(7,"Y");
      prepStatement.setString(8,"Y");
      prepStatement.setString(9,"N");
      prepStatement.setString(10, "48");
      prepStatement.setString(11, SiebelDelivery.Status.ENQUEUED.toString());

      prepStatement.execute();

    } finally {
      if (prepStatement != null) {
        prepStatement.close();
      }
    }
  }

  void createTestDelivery(int wid) throws SQLException{
    Connection connection = null;

    try {
      connection = ConnectionPool.getPool("default").getConnection();
      connection.setAutoCommit(false);

      _createDelivery(wid, connection);

      _createMessages(connection, wid);

      connection.commit();
      System.out.println("Completed:  wave");

    } finally {
      if(connection != null) {
        connection.setAutoCommit(true);
        connection.close();
      }
    }

  }
}
