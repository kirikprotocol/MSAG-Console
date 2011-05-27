package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import testutils.TestUtils;

import java.io.*;
import java.util.*;

import static org.junit.Assert.*;

/**
 * @author Aleksandr Khalitov
 */
public class MainLoopTaskTest {

  private MainLoopTask task;

  private File workDir;

  private SingleFileResource resource;

  private TestDeliveryManager deliveryManager;

  private User u;

  private static User prepareUser() {
    User u = new User();
    u.setLogin("a");
    u.setPassword("");
    u.setAllRegionsAllowed(true);
    return u;
  }

  @Before
  public void init() throws Exception{
    u = prepareUser();
    workDir = TestUtils.createRandomDir("-cpt");
    resource = new SingleFileResource();
    deliveryManager = new TestDeliveryManager();
    ContentProviderContext context = new SingleUserContentPContext(u, deliveryManager);
    task = new MainLoopTask(context, new UserDirResolver() {
      @Override
      public File getUserLocalDir(String login, UserCPsettings ucps) throws AdminException {
        return new File(workDir, login);
      }

      @Override
      public FileResource getConnection(User user, UserCPsettings ucps) throws AdminException {
        return resource;
      }
    }, workDir);

  }

  private UserCPsettings prepareSettings(UserCPsettings.WorkType workType) throws Exception{
    UserCPsettings settings = new UserCPsettings();
    settings.setActivePeriodEnd(new Time(0, 0, 0));
    settings.setActivePeriodStart(new Time(0, 0, 0));
    settings.setWorkType(workType);
    settings.setEncoding("UTF-8");
    settings.setSourceAddress(new Address("100"));
    return settings;
  }


  private int testCreated(String login) throws Exception{

    final boolean[] exist = new boolean[]{false};

    final int[] id = new int[1];
    deliveryManager.getDeliveries(login, "", new DeliveryFilter(), 1000, new Visitor<Delivery>() {
      @Override
      public boolean visit(Delivery value) throws AdminException {
        exist[0] = true;
        id[0] = value.getId();
        return false;
      }
    });
    assertTrue("Delivery hasn't been created", exist[0]);
    return id[0];

  }

  private int countMessages(String login, int id) throws Exception {
    MessageFilter filter = new MessageFilter(id, new Date(0), new Date(System.currentTimeMillis()+10000000));
    final int[] count = new int[]{0};
    deliveryManager.getMessages(login, "", filter, 1000, new Visitor<Message>() {
      @Override
      public boolean visit(Message value) throws AdminException {
        count[0]++;
        return true;
      }
    });
    return count[0];
  }

  private void _testSimple() throws Exception {

    resource.setDeliveryFile("test-cp-simple-" + System.currentTimeMillis() + ".csv");
    UserCPsettings settings = prepareSettings(UserCPsettings.WorkType.simple);

    ResourceProcessStrategy strategy = task.getStrategy(u, new File(workDir, u.getLogin()), settings, resource);

    strategy.process(true);

    try{
      resource.open();
      assertEquals(resource.listCSVFiles().size(), 0);
    }finally {
      resource.close();
    }
    int id = testCreated(u.getLogin());

    assertEquals(countMessages(u.getLogin(), id), 3);


    deliveryManager.dropDelivery(u.getLogin(),"", id);

  }

  private void _testDetailed(boolean createReports) throws Exception {
    String deliveryName = "test-cp-detailed"+System.currentTimeMillis();
    String remoteFile = deliveryName + ".csv";
    resource.setDeliveryFile(remoteFile);

    UserCPsettings settings = prepareSettings(UserCPsettings.WorkType.detailed);
    settings.setCreateReports(createReports);

    ResourceProcessStrategy strategy = task.getStrategy(u, new File(workDir, u.getLogin()), settings, resource);

    strategy.process(true);

    try{
      resource.open();
      assertEquals(resource.listCSVFiles().size(), 0);
    }finally {
      resource.close();
    }

    assertEquals(resource.deliveryFile, remoteFile+".active");

    int id = testCreated(u.getLogin());

    assertEquals(countMessages(u.getLogin(), id), 3);

    int tried = 0;
    do{
      Thread.sleep(500);
      deliveryManager.forceModifyDeliveries();
      tried ++;
      if(tried==100) {
        break;
      }
    }while (deliveryManager.getDelivery("","", id).getStatus() !=  DeliveryStatus.Finished);

    if(tried == 100) {
      fail("Something is wrong, delivery is not finished");
    }

    strategy.process(false);

    try{
      resource.open();
      assertEquals(resource.listCSVFiles().size(), 0);
    }finally {
      resource.close();
    }
    assertEquals(resource.deliveryFile, remoteFile+".finished");
  }


  @Test
  public void testDetailedNoReport() throws Exception {
    _testDetailed(false);
    assertEquals(resource.uploaded.size(), 0);
  }

  @Test
  public void testDetailedReport() throws Exception {
    _testDetailed(true);
    assertEquals(resource.uploaded.size(), 1);
  }


  @Test
  public void testSimple() throws Exception {
    _testSimple();
  }
  @Test
  public void testSingleTextDetailedNoReport() throws Exception {
    resource.singleText = true;
    _testDetailed(false);
    assertEquals(resource.uploaded.size(), 0);
  }

  @Test
  public void testSingleTextDetailedReport() throws Exception {
    resource.singleText = true;
    _testDetailed(true);
    assertEquals(resource.uploaded.size(), 1);
  }


  @Test
  public void testSingleTextSimple() throws Exception {
    resource.singleText = true;
    _testSimple();
  }


  @Test(expected = IllegalStateException.class)
  public void testOpenError() throws AdminException {
    resource.listCSVFiles();
  }

  @Test
  public void cleanAfterCrash() throws Exception{
    File parent = new File(workDir.getAbsolutePath() + File.separatorChar + u.getLogin());
    assertTrue("Can't create dir", parent.exists() || parent.mkdirs());
    File f1 = new File(parent, "test1.1111.gen");
    File f2 = new File(parent, "test2.1111.not.generated");
    assertTrue("Can't create files", f1.createNewFile() && f2.createNewFile());

    UserCPsettings settings = prepareSettings(UserCPsettings.WorkType.simple);
    ResourceProcessStrategy strategy = task.getStrategy(u, new File(workDir, u.getLogin()), settings, resource);
    strategy.process(true);

    assertFalse("File is not deleted", f1.exists());
    assertFalse("File is not deleted", f2.exists());

    assertEquals(deliveryManager.countDeliveries(u.getLogin(), "", new DeliveryFilter()), 0);

  }


  @After
  public void shutdown() {
    if(workDir != null) {
      TestUtils.recursiveDeleteFolder(workDir);
    }
  }


  private static class SingleFileResource extends FileResource {

    private String deliveryFile;

    private Set<String> uploaded = new HashSet<String>(5);

    private boolean opened;

    private boolean singleText = false;


    public String getDeliveryFile() {
      return deliveryFile;
    }

    public void setDeliveryFile(String deliveryFile) {
      this.deliveryFile = deliveryFile;
    }

    @Override
    public void open() throws AdminException {
      if(opened) {
        throw new IllegalStateException("Resource is already opened!");
      }
      opened = true;
    }

    @Override
    public List<String> listCSVFiles() throws AdminException {
      checkOpened();
      if(deliveryFile == null || !deliveryFile.endsWith(".csv")) {
        return Collections.emptyList();
      }
      List<String> result = new ArrayList<String>(1);
      result.add(deliveryFile);
      return result;
    }

    @Override
    public void get(String path, OutputStream os) throws AdminException {
      checkOpened();
      if(deliveryFile == null || !deliveryFile.equals(path)) {
        return;
      }
//      File parent = localFile.getParentFile();
//      if(parent != null && !parent.exists()) {
//        localFile.getParentFile().mkdirs();
//      }
      PrintWriter wr = null;
      try{
        wr = new PrintWriter(new OutputStreamWriter(os));
        wr.print("+79139489906|Hello \\n world!");wr.println(singleText ? "" : "1");
        wr.print("89139489907|Hello \\n world!");wr.println(singleText ? "" : "2");
        wr.print("79139489906|Hello \\n world!");wr.println(singleText ? "" : "3");
      }
      finally {
        if(wr != null) {
          wr.close();
        }
      }
    }

    @Override
    public void rename(String fromPath, String toPath) throws AdminException {
      checkOpened();
      if(deliveryFile == null || !deliveryFile.equals(fromPath)) {
        return;
      }
      deliveryFile = toPath;
    }

    @Override
    public void remove(String path) throws AdminException {
      checkOpened();
      if(deliveryFile == null || !deliveryFile.equals(path)) {
        return;
      }
      deliveryFile = null;
    }

    @Override
    public void put(InputStream is, String toPath) throws AdminException {
      checkOpened();
      uploaded.add(toPath);
    }

    @Override
    public void close() throws AdminException {
      checkOpened();
      opened = false;
    }

    private void checkOpened() {
      if(!opened) {
        throw new IllegalStateException("Resource is closed!");
      }
    }
  }

  private static class SingleUserContentPContext implements ContentProviderContext {

    private final User user;

    private final TestDeliveryManager deliveryManager;

    private SingleUserContentPContext(User user, TestDeliveryManager deliveryManager) {
      this.user = user;
      user.setAllRegionsAllowed(true);
      this.deliveryManager = deliveryManager;
    }

    @Override
    public FileSystem getFileSystem() {
      return new TestFileSystem();
    }

    @Override
    public List<User> getUsers() {
      List<User> us = new ArrayList<User>(1);
      us.add(user);
      return us;
    }

    @Override
    public User getUser(String login) {
      if(!user.getLogin().equals(login)) {
        return null;
      }
      return user;
    }

    @Override
    public Region getRegion(Address ab) {
      return null;
    }

    @Override
    public Delivery createDeliveryWithIndividualTexts(String login, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
      return deliveryManager.createDeliveryWithIndividualTexts(login, "", delivery, msDataSource);
    }

    @Override
    public Delivery createDeliveryWithSingleTextWithData(String login, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
      return deliveryManager.createDeliveryWithSingleTextWithData(login, "", delivery, msDataSource);
    }

    @Override
    public Delivery getDelivery(String login, int deliveryId) throws AdminException {
      return deliveryManager.getDelivery(login, "", deliveryId);
    }

    @Override
    public void activateDelivery(String login, int deliveryId) throws AdminException {
      deliveryManager.activateDelivery(login, "", deliveryId);
      deliveryManager.forceActivatePlannedDeliveries();
    }

    @Override
    public void pauseDelivery(String login, int deliveryId) throws AdminException {
      deliveryManager.pauseDelivery(login, "", deliveryId);
    }

    @Override
    public void dropDelivery(String login, int deliveryId) throws AdminException {
      deliveryManager.dropDelivery(login, "", deliveryId);
    }

    @Override
    public void modifyDelivery(String login, Delivery delivery) throws AdminException {
      deliveryManager.modifyDelivery(login, "", delivery);
    }

    @Override
    public void checkNoRestrictions(String login) throws AdminException {
      //To change body of implemented methods use File | Settings | File Templates.
    }

    @Override
    public void copyUserSettingsToDeliveryPrototype(String login, DeliveryPrototype delivery) throws AdminException {
      delivery.setOwner(login);

      delivery.setSourceAddress(new Address("100"));

      delivery.setDeliveryMode(DeliveryMode.SMS);

      delivery.setPriority(1);

      delivery.setActivePeriodStart(new Time(0,0,0));

      delivery.setActivePeriodEnd(new Time(23,59,59));

      List<Day> days = new ArrayList<Day>(7);
      for (int i=1; i<8;i++) {
        days.add(Day.valueOf(i));
      }
      delivery.setActiveWeekDays(days.toArray(new Day[days.size()]));

    }

    @Override
    public void addMessages(String login, DataSource<Message> messageSource, int deliveryId) throws AdminException {
      deliveryManager.addIndividualMessages(login, "", messageSource, deliveryId);
    }

    @Override
    public void addSingleMessagesWithData(String login, DataSource<Message> messageSource, int deliveryId) throws AdminException {
      deliveryManager.addSingleTextMessagesWithData(login, "", messageSource, deliveryId);
    }

    @Override
    public void getMessagesStates(String login, MessageFilter filter, int _pieceSize, Visitor<Message> visitor) throws AdminException {
      deliveryManager.getMessages(login, "", filter, _pieceSize, visitor);
    }

    @Override
    public File getFtpUserHomeDir(String login) throws AdminException {
      return null;
    }

    @Override
    public void getDeliveries(String login, DeliveryFilter deliveryFilter, Visitor<Delivery> visitor) throws AdminException {
      deliveryManager.getDeliveries(login,"", deliveryFilter, 1000, visitor);
    }
  }
}
