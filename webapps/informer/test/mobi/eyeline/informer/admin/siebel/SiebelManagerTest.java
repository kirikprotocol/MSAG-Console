package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.DeliveryInfo;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.regions.Region;
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

  private static final int TIMEOUT = 5;

  @BeforeClass
  public static void init() throws Exception {
    try{
      ConnectionPool p = ConnectionPool.getPool("default");
      if(p != null) {
        p.shutdown();
      }
    }catch (Exception e){}

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

    siebel = new TestSiebelManager(deliveries = new TestSiebelDeliveries(siebelUser), new SiebelRegionManager() {
      public Region getRegion(Address msisdn) throws AdminException {
        return null;
      }
    });

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
    filter.setStatusFilter(DeliveryStatus.Active, DeliveryStatus.Finished);
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
    filter.setStatusFilter(DeliveryStatus.Active, DeliveryStatus.Finished);
    deliveries.getDeliveries(siebelUser.getLogin(), siebelUser.getPassword(), filter, 1000,  new Visitor<DeliveryInfo>() {
      public boolean visit(DeliveryInfo value) throws AdminException {
        if(value.getName().startsWith("siebel_")) {
          exist[0] = true;
          return false;
        }
        return true;
      }
    });
    assertTrue("Delivery's status is not ACTIVATED", exist[0]);
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
    int wid = 10000;
    try{
      CreateDelivery.createDelivery(wid);
      Thread.sleep(3000*TIMEOUT);

      testCreation(wid);

      setSiebelState(wid, SiebelDelivery.Status.PAUSED);

      Thread.sleep(3000*TIMEOUT);

      testPaused(wid);

      setSiebelState(wid, SiebelDelivery.Status.ENQUEUED);

      Thread.sleep(3000*TIMEOUT);

      testActivated(wid);

      setSiebelState(wid, SiebelDelivery.Status.STOPPED);

      Thread.sleep(3000*TIMEOUT);

      testStopped(wid);

      setSiebelState(wid, SiebelDelivery.Status.ENQUEUED);

      Thread.sleep(3000*TIMEOUT);

      siebel.stop();

      Properties props = readProps();
      props.setProperty(SiebelManager.REMOVE_ON_STOP_PARAM, Boolean.TRUE.toString());
      props.setProperty(SiebelManager.TIMEOUT, Integer.toString(TIMEOUT));

      siebel.start(siebelUser, props);

      setSiebelState(wid, SiebelDelivery.Status.STOPPED);

      Thread.sleep(3000*TIMEOUT);

      testDeleted(wid);

    }finally {
      CreateDelivery.removeDelivery(wid);
    }
  }

  @AfterClass
  public static void shutdown() throws Exception{
    if(siebel != null) {
      siebel.stop();
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

}
