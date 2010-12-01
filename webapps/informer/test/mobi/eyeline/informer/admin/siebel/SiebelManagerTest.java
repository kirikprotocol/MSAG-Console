package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
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

import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;
import java.util.Properties;

import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelManagerTest {

  private static TestSiebelManager siebel;
  private static User siebelUser;

  private static TestSiebelDeliveries deliveries;

  private static final int TIMEOUT = 3;


  private static TestSiebelDataProvider dataProvider = new TestSiebelDataProvider();

  @BeforeClass
  public static void init() throws Exception {
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

    siebel = new TestSiebelManager(dataProvider, deliveries = new TestSiebelDeliveries(siebelUser), new SiebelRegionManager() {
      public Region getRegion(Address msisdn) throws AdminException {
        return null;
      }
    });

    Properties props = new Properties();
    props.setProperty(SiebelManager.REMOVE_ON_STOP_PARAM, Boolean.FALSE.toString());
    props.setProperty(SiebelManager.TIMEOUT, Integer.toString(TIMEOUT));


    siebel.start(siebelUser, props);
  }


  private void testCreation() throws Exception{
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(siebelUser.getLogin());
    filter.setStatusFilter(DeliveryStatus.Planned, DeliveryStatus.Active, DeliveryStatus.Finished);
    final boolean[] exist = new boolean[]{false};
    deliveries.getDeliveries(siebelUser.getLogin(), siebelUser.getPassword(), filter, 1000,  new Visitor<Delivery>() {
      public boolean visit(Delivery value) throws AdminException {
        if(value.getName().startsWith("siebel_")) {
          exist[0] = true;
          return false;
        }
        return true;
      }
    });
    assertTrue("Delivery hasn't been created", exist[0]);
  }

  private void testPaused() throws Exception{
    final boolean[] exist = new boolean[]{false};
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(siebelUser.getLogin());
    filter.setStatusFilter(DeliveryStatus.Paused);
    deliveries.getDeliveries(siebelUser.getLogin(), siebelUser.getPassword(), filter, 1000,  new Visitor<Delivery>() {
      public boolean visit(Delivery value) throws AdminException {
        if(value.getName().startsWith("siebel_")) {
          exist[0] = true;
          return false;
        }
        return true;
      }
    });
    assertTrue("Delivery's status is not PAUSED", exist[0]);
  }

  private void testActivated() throws Exception{
    final boolean[] exist = new boolean[]{false};
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(siebelUser.getLogin());
    filter.setStatusFilter(DeliveryStatus.Planned, DeliveryStatus.Active, DeliveryStatus.Finished);
    deliveries.getDeliveries(siebelUser.getLogin(), siebelUser.getPassword(), filter, 1000,  new Visitor<Delivery>() {
      public boolean visit(Delivery value) throws AdminException {
        if(value.getName().startsWith("siebel_")) {
          exist[0] = true;
          return false;
        }
        return true;
      }
    });
    assertTrue("Delivery's status is not ACTIVATED", exist[0]);
  }

  private void testStopped() throws Exception{
    final boolean[] exist = new boolean[]{false};
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(siebelUser.getLogin());
    filter.setStatusFilter(DeliveryStatus.Cancelled);
    deliveries.getDeliveries(siebelUser.getLogin(), siebelUser.getPassword(), filter, 1000,  new Visitor<Delivery>() {
      public boolean visit(Delivery value) throws AdminException {
        if(value.getName().startsWith("siebel_")) {
          exist[0] = true;
          return false;
        }
        return true;
      }
    });
    assertTrue("Delivery's status is not STOPPED", exist[0]);
  }

  private void testDeleted() throws Exception {
    final boolean[] exist = new boolean[]{false};
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(siebelUser.getLogin());
    filter.setStatusFilter(DeliveryStatus.Cancelled);
    deliveries.getDeliveries(siebelUser.getLogin(), siebelUser.getPassword(), filter, 1000,  new Visitor<Delivery>() {
      public boolean visit(Delivery value) throws AdminException {
        if(value.getName().startsWith("siebel_")) {
          exist[0] = true;
        }
        return true;
      }
    });
    assertTrue("Delivery's status hasn't been removed", !exist[0]);
  }

  private void createDelivery(String wid) {
    int i=0;
    SiebelMessage
        sm = new SiebelMessage();
    sm.setClcId(Integer.toString(++i));
    sm.setCreated(new Date());
    sm.setLastUpd(new Date());
    sm.setMsisdn("+79529223755");
    dataProvider.addMessage(wid, sm);
        sm = new SiebelMessage();
    sm.setClcId(Integer.toString(++i));
    sm.setCreated(new Date());
    sm.setLastUpd(new Date());
    sm.setMsisdn("+79139489906");
    dataProvider.addMessage(wid, sm);
        sm = new SiebelMessage();
    sm.setClcId(Integer.toString(++i));
    sm.setCreated(new Date());
    sm.setLastUpd(new Date());
    sm.setMsisdn("+79167543243");
    dataProvider.addMessage(wid, sm);

    SiebelDelivery sd = new SiebelDelivery();
    sd.setStatus(SiebelDelivery.Status.ENQUEUED);
    sd.setCreated(new Date());
    sd.setLastUpdate(new Date());
    sd.setExpPeriod(1);
    sd.setPriority(10);
    sd.setWaveId(wid);

    dataProvider.addDelivery(sd);
  }

  @Test
  public void testDelivery() throws Exception{
    String wid = "10000";

    createDelivery(wid);

    Thread.sleep(TIMEOUT*1500);

    testCreation();

    dataProvider.setDeliveryStatus(wid, SiebelDelivery.Status.PAUSED);

    Thread.sleep(TIMEOUT*1500);

    testPaused();

    dataProvider.setDeliveryStatus(wid, SiebelDelivery.Status.ENQUEUED);

    Thread.sleep(TIMEOUT*1500);

    testActivated();

    dataProvider.setDeliveryStatus(wid, SiebelDelivery.Status.STOPPED);

    Thread.sleep(TIMEOUT*1500);

    testStopped();

    dataProvider.setDeliveryStatus(wid, SiebelDelivery.Status.ENQUEUED);

    Thread.sleep(TIMEOUT*1500);

    siebel.stop();

    Properties props = new Properties();
    props.setProperty(SiebelManager.REMOVE_ON_STOP_PARAM, Boolean.TRUE.toString());
    props.setProperty(SiebelManager.TIMEOUT, Integer.toString(TIMEOUT));

    siebel.start(siebelUser, props);

    dataProvider.setDeliveryStatus(wid, SiebelDelivery.Status.STOPPED);

    Thread.sleep(TIMEOUT*1500);

    testDeleted();

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


}
