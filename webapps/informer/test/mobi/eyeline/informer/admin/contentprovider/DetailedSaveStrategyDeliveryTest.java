package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.MemoryFileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import org.junit.Test;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.List;

import static org.junit.Assert.*;

/**
 * User: artem
 * Date: 11.07.11
 */
public class DetailedSaveStrategyDeliveryTest {

  private MemoryFileSystem fs = new MemoryFileSystem();
  private ContentProviderContextStub ctx = new ContentProviderContextStub(fs);
  private FileResourceStub remoteResource = new FileResourceStub();

  private File workDir = new File("work");
  private File localCopyDir = new File(workDir, "detailedLocalCopy");

  private DetailedSaveStrategy createStrategy(ResourceOptions opts) throws AdminException {
    return new DetailedSaveStrategy(ctx, remoteResource, opts);
  }

  private DetailedSaveStrategy createStrategy(User u) throws AdminException {
    return new DetailedSaveStrategy(ctx, remoteResource, createResourceOptions(u));
  }

  private User createUser(String login) {
    User u = new User();
    u.setLogin(login);
    u.setAllRegionsAllowed(true);
    u.setDeliveryDays(Arrays.asList(0, 1, 2, 3, 4, 5, 6, 7));
    u.setSourceAddr(new Address(".5.0.MTC"));
    u.setPriority(1);
    u.setDeliveryStartTime(new Time(0, 0, 0));
    u.setDeliveryEndTime(new Time(23, 59, 59));
    u.setDeliveryType(User.DeliveryType.SMS);
    ctx.addUser(u);
    return u;
  }

  private ResourceOptions createResourceOptions(User u) {
    ResourceOptions opts = new ResourceOptions(u, workDir, new UserCPsettings());
    opts.setSourceAddress(new Address(".5.0.MTC"));
    return opts;
  }

  private Delivery getDelivery(String deliveryName, String user) throws AdminException {
    DeliveryFilter f = new DeliveryFilter();
    f.setNameFilter(deliveryName);
    final Delivery[] res = new Delivery[1];
    ctx.getDeliveries(user, f, new Visitor<Delivery>() {
      @Override
      public boolean visit(Delivery value) throws AdminException {
        res[0] = value;
        return false;
      }
    });
    return res[0];
  }

  private List<Delivery> getDeliveries(String deliveryName, String user) throws AdminException {
    DeliveryFilter f = new DeliveryFilter();
    f.setNameFilter(deliveryName);
    final List<Delivery> res = new ArrayList<Delivery>();
    ctx.getDeliveries(user, f, new Visitor<Delivery>() {
      @Override
      public boolean visit(Delivery value) throws AdminException {
        res.add(value);
        return true;
      }
    });
    return res;
  }

  private List<Message> getMessages(int deliveryId, String user) throws AdminException {
    MessageFilter f = new MessageFilter(deliveryId, new Date(0), new Date());
    final List<Message> result = new ArrayList<Message>();
    ctx.getMessagesStates(user, f, 100, new Visitor<Message>() {
      @Override
      public boolean visit(Message value) throws AdminException {
        result.add(value);
        return true;
      }
    });
    return result;
  }

  private void processResourceWithRandomUser(String login) throws AdminException {
    User u = createUser(login);
    ResourceOptions opts = createResourceOptions(u);
    DetailedSaveStrategy s = createStrategy(opts);

    s.process(true);
  }

  @Test
  public void testImportFileWithOneLine() throws AdminException {
    remoteResource.addFile("test.csv",
        "+79139995774|Hello world");

    processResourceWithRandomUser("login");

    Delivery d = getDelivery("test", "login");
    assertEquals(Delivery.Type.SingleText, d.getType());

    assertTrue(fs.exists(new File(localCopyDir, "test.csv.active")));
    remoteResource.open();
    assertTrue(remoteResource.contains("test.csv.active"));
    remoteResource.close();
  }

  @Test
  public void testImportFileWithEqualsTexts() throws AdminException {
    remoteResource.addFile("test.csv",
        "+79139495113|Hello world",
        "+79139995774|Hello world");

    processResourceWithRandomUser("login");

    Delivery d = getDelivery("test", "login");
    assertEquals(Delivery.Type.SingleText, d.getType());
    assertTrue(fs.exists(new File(localCopyDir, "test.csv.active")));
    remoteResource.open();
    assertTrue(remoteResource.contains("test.csv.active"));
    remoteResource.close();
  }

  @Test
  public void testImportFileWithDifferentTexts() throws AdminException {
    remoteResource.addFile("test.csv",
        "+79139495113|One text",
        "+79139995774|Another text");

    processResourceWithRandomUser("login");

    Delivery d = getDelivery("test", "login");
    assertEquals(Delivery.Type.IndividualTexts, d.getType());
    assertTrue(fs.exists(new File(localCopyDir, "test.csv.active")));
    remoteResource.open();
    assertTrue(remoteResource.contains("test.csv.active"));
    remoteResource.close();
  }

  @Test
  public void testImportEmptyFile() throws AdminException {
    remoteResource.addFile("test.csv");

    processResourceWithRandomUser("login");

    Delivery d = getDelivery("test", "login");
    assertEquals(Delivery.Type.IndividualTexts, d.getType());
    assertTrue(fs.exists(new File(localCopyDir, "test.csv.active")));
    remoteResource.open();
    assertTrue(remoteResource.contains("test.csv.active"));
    remoteResource.close();
  }

  @Test
  public void testImportFileWithActivationError() throws AdminException {
    remoteResource.addFile("test.csv", "+79139495113|One text");
    ctx.deprecateDeliveryActivation("test", 1);

    processResourceWithRandomUser("login");

    assertNotNull(getDelivery("test", "login"));

    assertEquals(DeliveryStatus.Paused, getDelivery("test", "login").getStatus());

    assertTrue(fs.exists(new File(localCopyDir, "test.csv")));
    remoteResource.open();
    assertTrue(remoteResource.contains("test.csv"));
    remoteResource.close();

    processResourceWithRandomUser("login");

    assertTrue(fs.exists(new File(localCopyDir, "test.csv.active")));
    remoteResource.open();
    assertTrue(remoteResource.contains("test.csv.active"));
    remoteResource.close();

    assertEquals(DeliveryStatus.Planned, getDelivery("test", "login").getStatus());
  }

  @Test
  public void testImportFileWithCreationError() throws AdminException {
    remoteResource.addFile("test.csv", "+79139495113|One text");
    ctx.deprecateDeliveryCreation("test", 1);

    processResourceWithRandomUser("login");

    assertNull(getDelivery("test", "login"));

    assertTrue(fs.exists(new File(localCopyDir, "test.csv")));
    remoteResource.open();
    assertTrue(remoteResource.contains("test.csv"));
    remoteResource.close();

    processResourceWithRandomUser("login");

    assertTrue(fs.exists(new File(localCopyDir, "test.csv.active")));
    remoteResource.open();
    assertTrue(remoteResource.contains("test.csv.active"));
    remoteResource.close();

    assertEquals(DeliveryStatus.Planned, getDelivery("test", "login").getStatus());
  }

  @Test
  public void testImportFileWithAddMessagesError() throws AdminException {
    remoteResource.addFile("test.csv", "+79139495113|One text");
    ctx.deprecateAddMessages("test", 1);

    processResourceWithRandomUser("login");

    assertNotNull(getDelivery("test", "login"));

    assertEquals(DeliveryStatus.Paused, getDelivery("test", "login").getStatus());

    assertTrue(fs.exists(new File(localCopyDir, "test.csv")));
    remoteResource.open();
    assertTrue(remoteResource.contains("test.csv"));
    remoteResource.close();

    processResourceWithRandomUser("login");

    assertTrue(fs.exists(new File(localCopyDir, "test.csv.active")));
    remoteResource.open();
    assertTrue(remoteResource.contains("test.csv.active"));
    remoteResource.close();

    assertEquals(DeliveryStatus.Planned, getDelivery("test", "login").getStatus());

  }

  private void assertFileContainsLine(File f, String pattern) throws AdminException, IOException {
    BufferedReader r = null;
    try {
      r = new BufferedReader(new InputStreamReader(fs.getInputStream(f)));

      String l;
      while((l = r.readLine()) != null) {
        System.out.println(l);
        if (l.matches(pattern))
          return;
      }
    } finally {
      if (r != null)
        r.close();
    }
    fail();
  }

  @Test
  public void testImportFileWithInvalidLines() throws AdminException, IOException {
    remoteResource.addFile("test.csv",
        "+79139495113|One text",
        "dksjfahlksdjhflaksdhfja");

    processResourceWithRandomUser("login");
    Delivery d = getDelivery("test", "login");

    assertNotNull(d);
    assertEquals(1, getMessages(d.getId(), "login").size());
  }

  @Test
  public void testImportFileWithInvalidAddress() throws AdminException, IOException {
    remoteResource.addFile("test.csv",
        "+79139495113|One text",
        "invalidAddress|One text");

    processResourceWithRandomUser("login");
    Delivery d = getDelivery("test", "login");

    assertNotNull(d);
    assertEquals(1, getMessages(d.getId(), "login").size());

    File reportFile = new File(localCopyDir, "test.csv.report");
    assertTrue(fs.exists(reportFile));
    String datePattern = "\\d\\d\\d\\d\\.\\d\\d\\.\\d\\d \\d\\d\\:\\d\\d\\:\\d\\d";
    assertFileContainsLine(reportFile, "invalidAddress\\|" + datePattern + "\\|Failed\\|9999");
  }

  @Test
  public void testImportFileWithNotAllowedRegion() throws AdminException, IOException {
    remoteResource.addFile("test.csv",
        "+79139495113|One text",
        "+79166666666|One text");

    Region r = new Region();
    ctx.addRegion(r, "+79139495113");

    User u = createUser("login");
    u.setAllRegionsAllowed(false);
    u.setRegions(Arrays.asList(r.getRegionId()));


    DetailedSaveStrategy s = createStrategy(u);
    s.process(true);


    Delivery d = getDelivery("test", "login");

    assertNotNull(d);
    assertEquals(1, getMessages(d.getId(), "login").size());

    File reportFile = new File(localCopyDir, "test.csv.report");
    assertTrue(fs.exists(reportFile));
    String datePattern = "\\d\\d\\d\\d\\.\\d\\d\\.\\d\\d \\d\\d\\:\\d\\d\\:\\d\\d";
    assertFileContainsLine(reportFile, "\\+79166666666\\|" + datePattern + "\\|Failed\\|9999");
  }


}
