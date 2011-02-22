package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.util.Date;

import static org.junit.Assert.*;

/**
 * @author Aleksandr Khalitov
 */
public class ArchiveManagerTest {

  private ArchiveManager manager;

  private TestDeliveryManager dm;

  private File resultDir;

  @Before
  public void before() throws InitException {
    resultDir = TestUtils.createRandomDir("-results");
    dm = new TestDeliveryManager();
    final RequestStorage rs = new RequestMemoryStorage();
    final FileSystem fs = new TestFileSystem();

    manager = new ArchiveManager(rs, new ArchiveContext() {
      @Override
      public User getUser(String login) {
        User u = new User();
        u.setLogin(login);
        u.setPassword("");
        return u;
      }
      @Override
      public UnmodifiableDeliveryManager getDeliveryManager() {
        return dm;
      }
      @Override
      public FileSystem getFileSystem() {
        return fs;
      }
    }, resultDir, 10, 10);
  }

  @After
  public void after() {
    if(resultDir != null) {
      TestUtils.recursiveDeleteFolder(resultDir);
    }
    manager.shutdown();
    if(dm != null) {
      dm.shutdown();
    }
  }

  @Test(expected = AdminException.class)
  public void checkDeliveriesEmptyName() throws AdminException {
    DeliveriesRequestPrototype request = createDeliveriesPrototype();
    request.setName(null);
    manager.createRequest("", request);
  }

  @Test(expected = AdminException.class)
  public void checkMessagesEmptyName() throws AdminException {
   MessagesRequestPrototype request = createMessagesPrototype();
    request.setName(null);
    manager.createRequest("", request);
  }

  @Test(expected = AdminException.class)
  public void checkMessagesEmptyFrom() throws AdminException {
   MessagesRequestPrototype request = createMessagesPrototype();
    request.setFrom(null);
    manager.createRequest("", request);
  }

  @Test(expected = AdminException.class)
  public void checkMessagesEmptyAddress() throws AdminException {
   MessagesRequestPrototype request = createMessagesPrototype();
    request.setAddress(null);
    manager.createRequest("", request);
  }





  private static DeliveriesRequestPrototype createDeliveriesPrototype() {
    DeliveriesRequestPrototype q = new DeliveriesRequestPrototype();
    q.setName("deliveries request");
    q.setFrom(new Date(0));
    return q;
  }


  private static MessagesRequestPrototype createMessagesPrototype() {
    MessagesRequestPrototype q = new MessagesRequestPrototype();
    q.setName("messages request");
    q.setFrom(new Date(0));
    q.setAddress(new Address("+79139489906"));
    return q;
  }

  @Test
  public void addDeliveriesCheck() throws AdminException {

    DeliveriesRequestPrototype q = createDeliveriesPrototype();

    DeliveriesRequest request = manager.createRequest("", q);

    assertNotNull(request.getId());

    assertNotNull(manager.getRequest(request.getId()));

  }
  @Test
  public void addMessagesCheck() throws AdminException {

    MessagesRequestPrototype q = createMessagesPrototype();

    MessagesRequest request = manager.createRequest("", q);

    assertNotNull(request.getId());

    assertNotNull(manager.getRequest(request.getId()));

  }

  @Test
  public void removeDeliveriesCheck() throws AdminException {

    DeliveriesRequestPrototype q = createDeliveriesPrototype();

    DeliveriesRequest request = manager.createRequest("", q);

    manager.removeRequest(request.getId());

    assertNull(manager.getRequest(request.getId()));

  }

  @Test
  public void removeMessagesCheck() throws AdminException {

    MessagesRequestPrototype q = createMessagesPrototype();

    MessagesRequest request = manager.createRequest("", q);

    manager.removeRequest(request.getId());

    assertNull(manager.getRequest(request.getId()));

  }

  @Test
  public void createdDeliveriesStatus() throws AdminException {
    DeliveriesRequestPrototype q = createDeliveriesPrototype();

    DeliveriesRequest request = manager.createRequest("", q);

    assertNotNull(manager.getRequest(request.getId()).getStatus());

    assertNotSame(manager.getRequest(request.getId()).getStatus(), DeliveriesRequest.Status.CANCELED);

  }
  @Test
  public void createdMessagesStatus() throws AdminException {
    MessagesRequestPrototype q = createMessagesPrototype();

   MessagesRequest request = manager.createRequest("", q);

    assertNotNull(manager.getRequest(request.getId()).getStatus());

    assertNotSame(manager.getRequest(request.getId()).getStatus(), DeliveriesRequest.Status.CANCELED);

  }


  @Test
  public void cancelDeliveries() throws AdminException {
    DeliveriesRequestPrototype q = createDeliveriesPrototype();
    DeliveriesRequest request = manager.createRequest("", q);
    manager.cancelRequest(request.getId());
    assertEquals(manager.getRequest(request.getId()).getStatus(), DeliveriesRequest.Status.CANCELED);
  }

  @Test
  public void cancelMessages() throws AdminException {
    DeliveriesRequestPrototype q = createDeliveriesPrototype();
    DeliveriesRequest request = manager.createRequest("", q);
    manager.cancelRequest(request.getId());
    assertEquals(manager.getRequest(request.getId()).getStatus(), DeliveriesRequest.Status.CANCELED);
  }

  private Delivery prepareDelivery() throws AdminException {
    DeliveryPrototype d = new DeliveryPrototype();
    d.setActivePeriodStart(new Time(1,0,0));
    d.setActivePeriodEnd(new Time(22, 0, 0));
    d.setActiveWeekDays(Day.values());
    d.setDeliveryMode(DeliveryMode.SMS);
    d.setEndDate(new Date(System.currentTimeMillis() + 300000));
    d.setStartDate(new Date(System.currentTimeMillis() - 300000));
    d.setName("Test delivery");
    d.setOwner("me");
    d.setPriority(15);
    d.setReplaceMessage(true);
    d.setRetryOnFail(true);
    d.setUseDataSm(true);
    d.setRetryPolicy("1h");
    d.setSvcType("svc1");
    d.setValidityPeriod(new Time(1, 0, 0));
    d.setSourceAddress(new Address("+79123942341"));
    d.setSingleText("text_text");
    d.setProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS, "dsada@dsadas.das");
    d.setProperty(UserDataConsts.SMS_NOTIF_ADDRESS, "+422414412");
    Delivery created = dm.createDeliveryWithSingleText("", "", d, new DataSource<Address>() {
      private boolean finish;
      @Override
      public Address next() throws AdminException {
        if (!finish) {
          finish = true;
          return new Address("+79139489906");
        }
        return null;
      }
    });
    dm.forceActivatePlannedDeliveries();
    dm.forceModifyDeliveries();
    return created;
  }

  private void assertDeliveries(Delivery d, ArchiveDelivery delivery) {
    assertEquals(delivery.getActivePeriodEnd(), d.getActivePeriodEnd());
    assertEquals(delivery.getActivePeriodStart(), d.getActivePeriodStart());
    assertArrayEquals(delivery.getActiveWeekDays(), d.getActiveWeekDays());
    assertEquals(delivery.getDeliveryMode(), d.getDeliveryMode());
    assertEquals(delivery.getEmailNotification(), d.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS));
    assertEquals(delivery.getSmsNotification().getSimpleAddress(), d.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS));
    assertEquals(delivery.getStartDate().getTime() / 1000, d.getStartDate().getTime() / 1000);
    assertEquals(delivery.getEndDate().getTime() / 1000, d.getEndDate().getTime() / 1000);
    assertEquals(delivery.getName(), d.getName());
    assertEquals(delivery.getId(), d.getId());
    assertEquals(delivery.getOwner(), d.getOwner());
    assertEquals(delivery.getPriority(), d.getPriority());
    assertEquals(delivery.getRetryPolicy(), d.getRetryPolicy());
    assertEquals(delivery.isRetryOnFail(), d.isRetryOnFail());
    assertEquals(delivery.isUseDataSm(), d.isUseDataSm());
    assertEquals(delivery.getSourceAddress(), d.getSourceAddress());
  }

  private void assertMessage(ArchiveMessage m, Delivery d) {
    assertEquals(m.getAbonent().getSimpleAddress(), "+79139489906");
    assertEquals(m.getOwner(), d.getOwner());
    assertEquals(m.getDeliveryId(), d.getId().intValue());
    assertEquals(m.getDeliveryName(), d.getName());
  }


  @Test
  public void checkDeliveriesResult() throws AdminException, InterruptedException {
    DeliveriesRequestPrototype q = createDeliveriesPrototype();
    DeliveriesRequest request = manager.createRequest("", q);

    Request r;
    long wait = 0;
    do {
      if(wait > 10000) {
        fail("Waiting period is too long");
      }
      Thread.sleep(500);
      wait+=1000;
      r = manager.getRequest(request.getId());
    }while (r.getStatus() != Request.Status.FINISHED);

    assertEquals(r.getProgress(), 100);
    assertNotNull(r.getStartDate());
    assertNotNull(r.getEndDate());
  }

  @Test
  public void checkMessagesResult() throws AdminException, InterruptedException {
    final Delivery d = prepareDelivery();

    MessagesRequestPrototype q = createMessagesPrototype();
    MessagesRequest request = manager.createRequest("", q);

    Request r;
    long wait = 0;
    do {
      if(wait > 10000) {
        fail("Waiting period is too long");
      }
      Thread.sleep(500);
      wait+=1000;
      r = manager.getRequest(request.getId());
    }while (r.getStatus() != Request.Status.FINISHED);

    assertEquals(r.getProgress(), 100);
    assertNotNull(r.getStartDate());
    assertNotNull(r.getEndDate());

    manager.getMessagesResult(r.getId(), new Visitor<ArchiveMessage>() {
      @Override
      public boolean visit(ArchiveMessage m) throws AdminException {
        assertMessage(m, d);
        return false;
      }
    });
  }

  @Test
  public void validateMessagesResult() throws AdminException, InterruptedException {
    final Delivery d = prepareDelivery();

    MessagesRequestPrototype q = createMessagesPrototype();
    MessagesRequest request = manager.createRequest("", q);

    Request r;
    long wait = 0;
    do {
      if(wait > 10000) {
        fail("Waiting period is too long");
      }
      Thread.sleep(500);
      wait+=1000;
      r = manager.getRequest(request.getId());
    }while (r.getStatus() != Request.Status.FINISHED);

    manager.getMessagesResult(r.getId(), new Visitor<ArchiveMessage>() {
      @Override
      public boolean visit(ArchiveMessage m) throws AdminException {
        assertMessage(m, d);
        return false;
      }
    });
  }

  @Test
  public void validateDeliveriesResult() throws AdminException, InterruptedException {
    final Delivery d = prepareDelivery();

    DeliveriesRequestPrototype q = createDeliveriesPrototype();
    DeliveriesRequest request = manager.createRequest("", q);

    Request r;
    long wait = 0;
    do {
      if(wait > 10000) {
        fail("Waiting period is too long");
      }
      Thread.sleep(500);
      wait+=1000;
      r = manager.getRequest(request.getId());
    }while (r.getStatus() != Request.Status.FINISHED);

    manager.getDeliveriesResult(r.getId(), new Visitor<ArchiveDelivery>() {
      @Override
      public boolean visit(ArchiveDelivery delivery) throws AdminException {
        assertDeliveries(d, delivery);
        return false;
      }
    });

  }


}
