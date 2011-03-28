package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.TestDelivery;
import mobi.eyeline.informer.admin.delivery.TestDeliveryFactory;
import mobi.eyeline.informer.admin.filesystem.MemoryFileSystem;
import mobi.eyeline.informer.admin.users.User;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.Assert.assertEquals;

/**
 * User: artem
 * Date: 28.03.11
 */
public class PerformRestrictionTest {

  private MemoryFileSystem fs;
  private File configFile;
  private File backupDir;
  private AtomicInteger deliveryId = new AtomicInteger(0);

  private List<RestrictionProvider> providers;

  @Before
  public void setUp() {
    fs = new MemoryFileSystem();
    configFile = new File("restrictions.csv");
    backupDir = new File("backup");
    providers = new ArrayList<RestrictionProvider>();
  }

  private RestrictionProvider createProvider(RestrictionContext ctx) throws Exception {
    RestrictionProvider p =  new RestrictionProvider(ctx, configFile, backupDir, fs);
    providers.add(p);
    return p;
  }

  @After
  public void tearDown() {
    for (RestrictionProvider p : providers)
      p.shutdown();
  }

  private static Restriction createRestriction(int id, String name, String from, String till, boolean all, String... users) throws Exception{
    Restriction r = new Restriction();
    r.setId(id);
    r.setName(name);
    r.setStartDate(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse(from));
    r.setEndDate(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse(till));
    r.setAllUsers(all);
    r.setUserIds(Arrays.asList(users));
    return r;
  }

  private Delivery createUserAndOneDeliveryForIt(String login, RestrictionContextMock ctx) {
    User u = new User();
    u.setLogin(login);
    u.setStatus(User.Status.ENABLED);

    ctx.addUser(u);

    TestDelivery d = TestDeliveryFactory.createDelivery();
    d.setId(deliveryId.incrementAndGet());
    d.setStatus(DeliveryStatus.Active);
    d.setOwner(u.getLogin());

    ctx.addDelivery(d);

    return d;
  }

  @Test
  /**
   * Проверяет, что ограничение устанавливает рассылку на паузу, а по окончании снова её активирует.
   * Тест для ограничения на всех пользователей
   */
  public void testExistingDeliveriesRestrictionForAllUsers() throws Exception {
    RestrictionContextMock ctx = new RestrictionContextMock();

    Delivery d1 = createUserAndOneDeliveryForIt("test1", ctx);
    Delivery d2 = createUserAndOneDeliveryForIt("test2", ctx);

    RestrictionProvider p = createProvider(ctx);
    ctx.resetPauseLatch(2);

    p.addRestriction(createRestriction(1, "test restriction", "2000.01.01 11:00:00", "2100.01.01 12:00:00", true));
    ctx.waitPauseLatch(2000);

    assertEquals(DeliveryStatus.Paused, d1.getStatus());
    assertEquals(DeliveryStatus.Paused, d2.getStatus());

    ctx.resetActivateLatch(2);
    p.updateRestriction(createRestriction(1, "test restriction", "2000.01.01 11:00:00", "2000.01.01 12:00:00", true));

    ctx.waitActivateLatch(2000);
    assertEquals(DeliveryStatus.Active, d1.getStatus());
    assertEquals(DeliveryStatus.Active, d2.getStatus());
  }

  @Test
  /**
   * Проверяет, что ограничение устанавливает рассылку на паузу, а по окончании снова её активирует
   * Тест на ограничение для одного пользователя
   */
  public void testExistingDeliveriesRestrictionForSelectedUsers() throws Exception {
    RestrictionContextMock ctx = new RestrictionContextMock();

    Delivery d1 = createUserAndOneDeliveryForIt("test1", ctx);
    Delivery d2 = createUserAndOneDeliveryForIt("test2", ctx);

    RestrictionProvider p = createProvider(ctx);
    ctx.resetPauseLatch(1);

    p.addRestriction(createRestriction(1, "test restriction", "2000.01.01 11:00:00", "2100.01.01 12:00:00", false, "test1"));
    ctx.waitPauseLatch(2000);

    assertEquals(DeliveryStatus.Paused, d1.getStatus());
    assertEquals(DeliveryStatus.Active, d2.getStatus());

    ctx.resetActivateLatch(1);
    p.updateRestriction(createRestriction(1, "test restriction", "2000.01.01 11:00:00", "2000.01.01 12:00:00", true));

    ctx.waitActivateLatch(2000);
    assertEquals(DeliveryStatus.Active, d1.getStatus());
    assertEquals(DeliveryStatus.Active, d2.getStatus());
  }

  @Test
  /**
   * Проверяет, что рассылки активируются после того, как ограничение удаляется
   * Тест на ограничение для всех пользователей
   */
  public void testDeliveryActivationOnRestrictionDeleteForAllUsers() throws Exception {
    RestrictionContextMock ctx = new RestrictionContextMock();

    Delivery d1 = createUserAndOneDeliveryForIt("test1",ctx);
    Delivery d2 = createUserAndOneDeliveryForIt("test2",ctx);

    RestrictionProvider p = createProvider(ctx);
    ctx.resetPauseLatch(2);

    p.addRestriction(createRestriction(1, "test restriction", "2000.01.01 11:00:00", "2100.01.01 12:00:00", true));
    ctx.waitPauseLatch(2000);

    assertEquals(DeliveryStatus.Paused, d1.getStatus());
    assertEquals(DeliveryStatus.Paused, d2.getStatus());

    ctx.resetActivateLatch(2);
    p.deleteRestriction(1);

    ctx.waitActivateLatch(2000);
    assertEquals(DeliveryStatus.Active, d1.getStatus());
    assertEquals(DeliveryStatus.Active, d2.getStatus());
  }

  @Test
  /**
   * Проверяет, что рассылки активируются после того, как ограничение удаляется
   * Тест на ограничение для одного пользователя
   */
  public void testDeliveryActivationOnRestrictionDeleteForSelectedUsers() throws Exception {
    RestrictionContextMock ctx = new RestrictionContextMock();

    Delivery d1 = createUserAndOneDeliveryForIt("test1",ctx);
    Delivery d2 = createUserAndOneDeliveryForIt("test2",ctx);

    RestrictionProvider p = createProvider(ctx);
    ctx.resetPauseLatch(1);

    p.addRestriction(createRestriction(1, "test restriction", "2000.01.01 11:00:00", "2100.01.01 12:00:00", false, "test1"));
    ctx.waitPauseLatch(2000);

    assertEquals(DeliveryStatus.Paused, d1.getStatus());
    assertEquals(DeliveryStatus.Active, d2.getStatus());

    ctx.resetActivateLatch(1);
    p.deleteRestriction(1);

    ctx.waitActivateLatch(2000);
    assertEquals(DeliveryStatus.Active, d1.getStatus());
    assertEquals(DeliveryStatus.Active, d2.getStatus());
  }


}
