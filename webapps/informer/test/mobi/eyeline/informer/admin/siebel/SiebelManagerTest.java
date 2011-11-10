package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListener;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetector;
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

import static org.junit.Assert.*;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelManagerTest {

  private static TestSiebelManager siebel;
  private static User siebelUser;

  private static TestSiebelDeliveries deliveries;

  private static final int TIMEOUT = 3;


  private static TestSiebelDataProvider dataProvider = new TestSiebelDataProvider();

  private static Properties buildProps() {
    Properties props = new Properties();
    props.setProperty("removeOnStop", Boolean.FALSE.toString());
    props.setProperty("timeout", Integer.toString(TIMEOUT));
    props.setProperty("statsPeriod", "30");
    props.setProperty("jdbc.pool.type", "");
    props.setProperty("jdbc.user", "");
    props.getProperty("jdbc.source", "");
    props.setProperty("siebelUser", "");
    props.setProperty("jdbc.password", "");
    props.setProperty("validityPeriod.def", "03:00:00");
    props.setProperty("validityPeriod.min", "02:00:00");
    props.setProperty("validityPeriod.max", "12:00:00");
    props.setProperty("priority.min","2");
    props.setProperty("priority.max","1000");
    return props;
  }

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

    siebel = new TestSiebelManager(dataProvider, deliveries = new TestSiebelDeliveries(siebelUser, new DeliveryChangesDetector() {
      @Override
      public void addListener(DeliveryChangeListener listener) {
      }
      @Override
      public void removeListener(DeliveryChangeListener listener) {
      }
      @Override
      public void removeAllListeners() {
      }
    }));
    siebel.start(siebelUser.getLogin(), new SiebelSettings(buildProps()));
  }


  private void testCreation() throws Exception{
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(siebelUser.getLogin());
    filter.setStatusFilter(DeliveryStatus.Planned, DeliveryStatus.Active, DeliveryStatus.Finished);
    final boolean[] exist = new boolean[]{false};
    deliveries.getDeliveries(siebelUser.getLogin(), filter, new Visitor<Delivery>() {
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
    deliveries.getDeliveries(siebelUser.getLogin(), filter,  new Visitor<Delivery>() {
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
    deliveries.getDeliveries(siebelUser.getLogin(), filter, new Visitor<Delivery>() {
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
    deliveries.getDeliveries(siebelUser.getLogin(), filter, new Visitor<Delivery>() {
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
    deliveries.getDeliveries(siebelUser.getLogin(), filter, new Visitor<Delivery>() {
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
    sm.setMessage("message");
    dataProvider.addMessage(wid, sm);
    sm = new SiebelMessage();
    sm.setClcId(Integer.toString(++i));
    sm.setCreated(new Date());
    sm.setLastUpd(new Date());
    sm.setMsisdn("+79167543243");
    dataProvider.addMessage(wid, sm);
    sm = new SiebelMessage();
    sm.setClcId(Integer.toString(++i));
    sm.setCreated(new Date());
    sm.setLastUpd(new Date());
    sm.setMsisdn("dasdsaasddasasd");
    sm.setMessage("Message");
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

  private void testMessages() throws AdminException {
    int i=0;
    SiebelMessage sm = dataProvider.getMessage(Integer.toString(++i));
    assertNotNull(sm);
    assertNotSame(sm.getSmscCode(),"11");
    assertNotSame(sm.getSmscValue(),"Invalid Dest Addr");
    sm = dataProvider.getMessage(Integer.toString(++i));
    assertNotNull(sm);
    assertNotSame(sm.getSmscCode(),"11");
    assertNotSame(sm.getSmscValue(),"Invalid Dest Addr");
    assertEquals(sm.getMessageState(), DeliveryMessageState.State.REJECTED.toString());
    sm = dataProvider.getMessage(Integer.toString(++i));
    assertNotNull(sm);
    assertEquals(sm.getSmscCode(),"11");
    assertEquals(sm.getSmscValue(),"Invalid Dest Addr");
    assertEquals(sm.getMessageState(), DeliveryMessageState.State.REJECTED.toString());
  }

  @Test
  public void testDelivery() throws Exception{
    String wid = "10000";

    createDelivery(wid);

    Thread.sleep(TIMEOUT*1500);

    testCreation();
    testMessages();

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

    Properties props = buildProps();
    props.setProperty("removeOnStop", Boolean.TRUE.toString());

    siebel.start(siebelUser.getLogin(), new SiebelSettings(props));

    dataProvider.setDeliveryStatus(wid, SiebelDelivery.Status.STOPPED);

    Thread.sleep(TIMEOUT*1500);

    testDeleted();

  }


  @Test
  public void testValidityPeriod() throws AdminException {
    SiebelDelivery siebelDelivery = new SiebelDelivery();
    siebelDelivery.setWaveId("wave123");

    siebelDelivery.setExpPeriod(null);
    DeliveryPrototype p = siebel.createDelivery(siebelDelivery, "test");
    assertEquals(p.getValidityPeriod().toString(), "03:00:00");

    siebelDelivery.setExpPeriod(1);
    p = siebel.createDelivery(siebelDelivery, "test");
    assertEquals(p.getValidityPeriod().toString(), "02:00:00");

    siebelDelivery.setExpPeriod(124);
    p = siebel.createDelivery(siebelDelivery, "test");
    assertEquals(p.getValidityPeriod().toString(), "12:00:00");
  }


  @Test
  public void testPriority() throws AdminException {
    SiebelDelivery siebelDelivery = new SiebelDelivery();
    siebelDelivery.setWaveId("wave123");

    siebelDelivery.setPriority(1);

    DeliveryPrototype p = siebel.createDelivery(siebelDelivery, "test");
    assertEquals(p.getPriority(), 2);

    siebelDelivery.setPriority(124312321);
    p = siebel.createDelivery(siebelDelivery, "test");
    assertEquals(p.getPriority(), 1000);
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
