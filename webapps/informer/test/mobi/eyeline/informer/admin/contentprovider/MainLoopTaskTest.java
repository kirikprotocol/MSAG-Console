package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetector;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.*;
import java.util.*;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class MainLoopTaskTest {

  private static MainLoopTask task;

  private static File workFile;

  private static ContentProviderContext context;

  private static SingleFileResource resource;

  private static TestDeliveryManager deliveryManager;

  private static User prepareUser() {
    User u = new User();
    u.setLogin("a");
    u.setPassword("");
    u.setAllRegionsAllowed(true);
    return u;
  }

  @BeforeClass
  public static void init() throws Exception{
    workFile = TestUtils.createRandomDir("-cpt");
    resource = new SingleFileResource();
    deliveryManager = new TestDeliveryManager(null, null);
    context = new SingleUserContentPContext(prepareUser(), deliveryManager);
    task = new MainLoopTask(context, new UserDirResolver() {
      @Override
      public File getUserLocalDir(String login, UserCPsettings ucps) throws AdminException {
        return new File(workFile, login);
      }

      @Override
      public FileResource getConnection(User user, UserCPsettings ucps) throws AdminException {
        return resource;
      }
    }, workFile);

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

  @Test
  public void testSimple() throws Exception {

    resource.setDeliveryFile("test-cp-simple-" + System.currentTimeMillis() + ".csv");

    User u = prepareUser();
    UserCPsettings settings = prepareSettings(UserCPsettings.WorkType.simple);

    ResourceProcessStrategy strategy = task.getStrategy(settings);

    strategy.process(u, new File(workFile, u.getLogin()), settings);

    assertEquals(resource.listCSVFiles().size(), 0);

    int id = testCreated(u.getLogin());

    assertEquals(countMessages(u.getLogin(), id), 1);


    deliveryManager.dropDelivery(u.getLogin(),"", id);

  }


  @Test
  public void testDetailed() throws Exception {
    String deliveryName = "test-cp-detailed"+System.currentTimeMillis();
    String remoteFile = deliveryName + ".csv";
    resource.setDeliveryFile(remoteFile);

    User u = prepareUser();
    UserCPsettings settings = prepareSettings(UserCPsettings.WorkType.detailed);

    ResourceProcessStrategy strategy = task.getStrategy(settings);

    strategy.process(u, new File(workFile, u.getLogin()), settings);

    assertEquals(resource.listCSVFiles().size(), 0);
    assertEquals(resource.deliveryFile, remoteFile+".active");

    int id = testCreated(u.getLogin());

    assertEquals(countMessages(u.getLogin(), id), 1);

    deliveryManager.forceModifyDeliveries();

    File finished = new File(workFile, u.getLogin()+File.separatorChar+deliveryName+".fin");
    File report = new File(workFile, u.getLogin()+File.separatorChar+deliveryName+".report");


    finished.createNewFile();
    report.createNewFile();


    strategy.process(u, new File(workFile, u.getLogin()), settings);


    assertEquals(resource.listCSVFiles().size(),0);
    assertEquals(resource.deliveryFile, remoteFile+".finished");

    assertEquals(resource.uploaded.size(), 1);



    deliveryManager.dropDelivery(u.getLogin(),"", id);
  }



  @AfterClass
  public static void shutdown() {
    if(workFile != null) {
      TestUtils.recursiveDeleteFolder(workFile);
    }
  }


  private static class SingleFileResource implements FileResource {

    private String deliveryFile;

    private Set<String> uploaded = new HashSet<String>(5);


    public String getDeliveryFile() {
      return deliveryFile;
    }

    public void setDeliveryFile(String deliveryFile) {
      this.deliveryFile = deliveryFile;
    }

    @Override
    public void open() throws AdminException {}

    @Override
    public List<String> listCSVFiles() throws AdminException {
      if(deliveryFile == null || !deliveryFile.endsWith(".csv")) {
        return Collections.emptyList();
      }
      List<String> result = new ArrayList<String>(1);
      result.add(deliveryFile);
      return result;
    }

    @Override
    public void get(String path, File localFile) throws AdminException {
      if(deliveryFile == null || !deliveryFile.equals(path)) {
        return;
      }
      File parent = localFile.getParentFile();
      if(parent != null && !parent.exists()) {
        localFile.getParentFile().mkdirs();
      }
      PrintWriter wr = null;
      try{
        wr = new PrintWriter(new BufferedWriter(new FileWriter(localFile)));
        wr.println("+79139489906|Hello, world!");
      }catch (IOException e){
        e.printStackTrace();
        throw new ContentProviderException("ioerror");
      }
      finally {
        if(wr != null) {
          wr.close();
        }
      }
    }

    @Override
    public void rename(String fromPath, String toPath) throws AdminException {
      if(deliveryFile == null || !deliveryFile.equals(fromPath)) {
        return;
      }
      deliveryFile = toPath;
    }

    @Override
    public void remove(String path) throws AdminException {
      if(deliveryFile == null || !deliveryFile.equals(path)) {
        return;
      }
      deliveryFile = null;
    }

    @Override
    public void put(File localFile, String toPath) throws AdminException {
      uploaded.add(toPath);
    }

    @Override
    public void close() throws AdminException {
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
      return deliveryManager.createDeliveryWithIndividualTexts(login,"", delivery, msDataSource);
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
    public void dropDelivery(String login, int deliveryId) throws AdminException {
      deliveryManager.dropDelivery(login, "", deliveryId);
    }

    @Override
    public void copyUserSettingsToDeliveryPrototype(String login, DeliveryPrototype delivery) throws AdminException {
      delivery.setOwner(login);

      delivery.setSourceAddress(new Address("100"));

      delivery.setDeliveryMode(DeliveryMode.SMS);

      delivery.setPriority(1);

      delivery.setActivePeriodStart(new Time(0,0,0));

      delivery.setActivePeriodEnd(new Time(0,0,0));

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
    public void getMessagesStates(String login, MessageFilter filter, int deliveryId, Visitor<Message> visitor) throws AdminException {
      filter.setDeliveryId(deliveryId);
      deliveryManager.getMessages(login, "", filter, 1000, visitor);
    }

    @Override
    public DeliveryChangesDetector getDeliveryChangesDetector() {
      return null;
    }
  }
}
