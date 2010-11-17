package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.siebel.impl.SiebelDeliveries;
import mobi.eyeline.informer.admin.siebel.impl.SiebelRegionManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import ru.sibinco.util.conpool.ConnectionPool;

import java.io.InputStream;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Locale;
import java.util.Properties;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelManagerTest {

  private static TestSiebelManager siebel;
  private static User siebelUser;

  private static TestSiebelDeliveries deliveries;

  private static final int TIMEOUT = 10;

  @BeforeClass
  public static void init() throws Exception {
    siebel = new TestSiebelManager(deliveries = new TestSiebelDeliveries(), new SiebelRegionManager() {
      public Region getRegion(Address msisdn) throws AdminException {
        return null;
      }
    });

    siebelUser = new User();
    siebelUser.setAllRegionsAllowed(true);
    siebelUser.setLogin("siebel");
    siebelUser.setPassword("siebel");

    siebelUser.setStatus(User.Status.ENABLED);
    siebelUser.setLocale(new Locale("en"));
    siebelUser.setSourceAddr(new Address("+79130000000"));
    siebelUser.setDeliveryDays(new ArrayList<Integer>(){{for(int i=0;i<7;i++){add(i);}}});
    siebelUser.setDeliveryStartTime(new Time(8,0,0));
    siebelUser.setDeliveryEndTime(new Time(20,0,0));
    siebelUser.setDeliveryType(User.DeliveryType.SMS);

    Properties props = readProps();
    props.setProperty(SiebelManager.REMOVE_ON_STOP_PARAM, Boolean.FALSE.toString());
    props.setProperty(SiebelManager.TIMEOUT, Integer.toString(TIMEOUT));


    siebel.start(siebelUser, props);
  }

  private static Properties readProps() throws Exception{
    Properties props = new Properties();
    InputStream is = null;
    try{
      is = SiebelManagerTest.class.getResourceAsStream("jdbc.properties");
      props.load(is);
    }finally {
      if(is != null) {
        try{
          is.close();
        }catch (Exception ignored){}
      }
    }
    return props;
  }

  private void testCreation(int wid) throws Exception{
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(siebelUser.getLogin());
    filter.setStatusFilter(DeliveryStatus.Active);
    final boolean[] exist = new boolean[]{false};
    deliveries.getDeliveries(siebelUser.getLogin(), siebelUser.getPassword(), filter, 1000,  new Visitor<DeliveryInfo>() {
      public boolean visit(DeliveryInfo value) throws AdminException {
        if(value.getName().startsWith("siebel_")) {
          exist[0] = true;
          return false;
        }
        return true;
      }
    });
    assertTrue("Delivery hasn't been created", exist[0]);
    assertEquals(SiebelDelivery.Status.IN_PROCESS, getSiebelState(wid));
  }

  private void testPaused(int wid) throws Exception{
    final boolean[] exist = new boolean[]{false};
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(siebelUser.getLogin());
    filter.setStatusFilter(DeliveryStatus.Paused);
    deliveries.getDeliveries(siebelUser.getLogin(), siebelUser.getPassword(), filter, 1000,  new Visitor<DeliveryInfo>() {
      public boolean visit(DeliveryInfo value) throws AdminException {
        if(value.getName().startsWith("siebel_")) {
          exist[0] = true;
          return false;
        }
        return true;
      }
    });
    assertTrue("Delivery's status is not PAUSED", exist[0]);
    assertEquals(SiebelDelivery.Status.PAUSED, getSiebelState(wid));
  }

  private void testActivated(int wid) throws Exception{
    final boolean[] exist = new boolean[]{false};
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(siebelUser.getLogin());
    filter.setStatusFilter(DeliveryStatus.Active);
    deliveries.getDeliveries(siebelUser.getLogin(), siebelUser.getPassword(), filter, 1000,  new Visitor<DeliveryInfo>() {
      public boolean visit(DeliveryInfo value) throws AdminException {
        if(value.getName().startsWith("siebel_")) {
          exist[0] = true;
          return false;
        }
        return true;
      }
    });
    assertTrue("Delivery's status is not PAUSED", exist[0]);
    assertEquals(SiebelDelivery.Status.IN_PROCESS, getSiebelState(wid));
  }

  private void testStopped(int wid) throws Exception{
    final boolean[] exist = new boolean[]{false};
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(siebelUser.getLogin());
    filter.setStatusFilter(DeliveryStatus.Cancelled);
    deliveries.getDeliveries(siebelUser.getLogin(), siebelUser.getPassword(), filter, 1000,  new Visitor<DeliveryInfo>() {
      public boolean visit(DeliveryInfo value) throws AdminException {
        if(value.getName().startsWith("siebel_")) {
          exist[0] = true;
          return false;
        }
        return true;
      }
    });
    assertTrue("Delivery's status is not STOPPED", exist[0]);
    assertEquals(SiebelDelivery.Status.STOPPED, getSiebelState(wid));
  }

  private void testDeleted(int wid) throws Exception {
    final boolean[] exist = new boolean[]{false};
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(siebelUser.getLogin());
    filter.setStatusFilter(DeliveryStatus.Cancelled);
    deliveries.getDeliveries(siebelUser.getLogin(), siebelUser.getPassword(), filter, 1000,  new Visitor<DeliveryInfo>() {
      public boolean visit(DeliveryInfo value) throws AdminException {
        if(value.getName().startsWith("siebel_")) {
          exist[0] = true;
        }
        return true;
      }
    });
    assertTrue("Delivery's status hasn't been removed", !exist[0]);
  }

  @Test
  public void testDelivery() throws Exception{
    int wid = (int)System.currentTimeMillis();
    try{
      siebel.createTestDelivery(wid);
      Thread.sleep(1000*TIMEOUT+5000);

      testCreation(wid);

      setSiebelState(wid, SiebelDelivery.Status.PAUSED);

      Thread.sleep(1000*TIMEOUT+5000);

      testPaused(wid);

      setSiebelState(wid, SiebelDelivery.Status.ENQUEUED);

      Thread.sleep(1000*TIMEOUT+5000);

      testActivated(wid);

      setSiebelState(wid, SiebelDelivery.Status.STOPPED);

      Thread.sleep(1000*TIMEOUT+5000);

      testStopped(wid);

      setSiebelState(wid, SiebelDelivery.Status.ENQUEUED);

      Thread.sleep(1000*TIMEOUT+5000);

      siebel.stop();

      Properties props = readProps();
      props.setProperty(SiebelManager.REMOVE_ON_STOP_PARAM, Boolean.TRUE.toString());
      props.setProperty(SiebelManager.TIMEOUT, Integer.toString(TIMEOUT));

      siebel.start(siebelUser, props);

      setSiebelState(wid, SiebelDelivery.Status.STOPPED);

      Thread.sleep(1000*TIMEOUT+5000);

      testDeleted(wid);

    }finally {
      removeAll(wid);
    }
  }

  @AfterClass
  public static void shutdown() throws Exception{
    if(siebel != null) {
      try {
        siebel.stop();
      } catch (AdminException ignored) {}
    }

    if(deliveries != null) {
      deliveries.shutdown();
    }
  }


  private static void setSiebelState(int wid, SiebelDelivery.Status status) throws SQLException {
    Connection connection = null;

    try {
      connection = ConnectionPool.getPool("default").getConnection();

      PreparedStatement prepStatement = null;
      try{
        prepStatement = connection.prepareStatement("update SMS_MAIL_PARAMS set CTRL_STATUS=? where WAVE_INT_ID=?");
        prepStatement.setString(1, status.toString());
        prepStatement.setString(2, Integer.toString(wid));

        prepStatement.executeUpdate();

      }finally {
        if(prepStatement != null) {
          prepStatement.close();
        }
      }


    } finally {
      if(connection != null) {
        try{
          connection.close();
        }catch (SQLException ignored){}
      }
    }
  }

  private static SiebelDelivery.Status getSiebelState(int wid) throws SQLException {
    Connection connection = null;

    try {
      connection = ConnectionPool.getPool("default").getConnection();

      PreparedStatement prepStatement = null;
      try{
        prepStatement = connection.prepareStatement("select CTRL_STATUS from  SMS_MAIL_PARAMS where WAVE_INT_ID=?");
        prepStatement.setString(1, Integer.toString(wid));

        java.sql.ResultSet rs = null;
        try{
          rs = prepStatement.executeQuery();
          if(rs.next()) {
            return SiebelDelivery.Status.valueOf(rs.getString(1));
          }else {
            return null;
          }

        }finally {
          if(rs != null) {
            try{
              rs.close();
            }catch (SQLException ignored){}
          }
        }
      }finally {
        if(prepStatement != null) {
          prepStatement.close();
        }
      }


    } finally {
      if(connection != null) {
        try{
          connection.close();
        }catch (SQLException ignored){}
      }
    }
  }

  
  private static void removeAll(int wid) throws Exception{

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

  private static class TestSiebelDeliveries implements SiebelDeliveries {
    private TestDeliveryManager deliveryManager = new TestDeliveryManager(null);

    public void addMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId) throws AdminException {
      deliveryManager.addMessages(login, password, msDataSource, deliveryId);
    }
    public void createDelivery(String login, String password, Delivery delivery, DataSource<Message> msDataSource) throws AdminException {
      deliveryManager.createDelivery(login, password, delivery, msDataSource);
    }

    public void dropDelivery(String login, String password, int deliveryId) throws AdminException {
      deliveryManager.dropDelivery(login, password, deliveryId);
    }


    public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException {
      return deliveryManager.getDelivery(login, password, deliveryId);
    }

    public void modifyDelivery(String login, String password, Delivery delivery) throws AdminException {
      deliveryManager.modifyDelivery(login, password, delivery);
    }

    public void cancelDelivery(String login, String password, int deliveryId) throws AdminException {
      deliveryManager.cancelDelivery(login, password, deliveryId);
    }

    public void pauseDelivery(String login, String password, int deliveryId) throws AdminException {
      deliveryManager.pauseDelivery(login, password, deliveryId);
    }

    public void activateDelivery(String login, String password, int deliveryId) throws AdminException {
      deliveryManager.activateDelivery(login, password, deliveryId);
    }


    public void getDeliveries(String login, String password, DeliveryFilter deliveryFilter, int _pieceSize, Visitor<DeliveryInfo> visitor) throws AdminException {
      deliveryManager.getDeliveries(login, password, deliveryFilter, _pieceSize, visitor);
    }

    public void getDefaultDelivery(String user, Delivery delivery) throws AdminException {
      AdminContext.getDefaultDelivery(siebelUser, delivery);
    }

    public void shutdown() {
      deliveryManager.shutdown();
    }
  }
}
