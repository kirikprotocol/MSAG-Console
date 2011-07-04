package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.filesystem.MemoryFileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;

import java.io.*;
import java.util.*;

import static org.junit.Assert.*;

/**
 * @author Aleksandr Khalitov
 */
@Ignore
public class MainLoopTaskTest {

  private MainLoopTask task;

  private File workDir;

  private SingleFileResource resource;

  private TestDeliveryManager deliveryManager;

  private User u;

  private FileSystem fileSystem;

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
    fileSystem = new MemoryFileSystem();
    fileSystem.createNewFile(workDir = new File("cpt-test-"+System.currentTimeMillis()));;
    resource = new SingleFileResource();
    deliveryManager = new TestDeliveryManager();
    ContentProviderContext context = new SingleUserContentPContext(u, deliveryManager, fileSystem);
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
    if(!fileSystem.exists(parent)) {
      fileSystem.mkdirs(parent);
    }
    File f1 = new File(parent, "test1.1111.gen");
    File f2 = new File(parent, "test2.1111.not.generated");

    fileSystem.createNewFile(f1);
    fileSystem.createNewFile(f2);

    UserCPsettings settings = prepareSettings(UserCPsettings.WorkType.simple);
    ResourceProcessStrategy strategy = task.getStrategy(u, new File(workDir, u.getLogin()), settings, resource);
    strategy.process(true);

    assertFalse("File is not deleted", f1.exists());
    assertFalse("File is not deleted", f2.exists());

    assertEquals(deliveryManager.countDeliveries(u.getLogin(), "", new DeliveryFilter()), 0);

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
    public boolean contains(String path) throws AdminException {
      return deliveryFile != null && deliveryFile.equals(path);
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

}
