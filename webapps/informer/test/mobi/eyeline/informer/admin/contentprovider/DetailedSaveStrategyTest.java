package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.MemoryFileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.BufferedWriter;
import java.io.File;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.Date;

import static junit.framework.Assert.assertEquals;
import static junit.framework.Assert.assertNull;
import static org.junit.Assert.*;

/**
 * author: Aleksandr Khalitov
 */
public class DetailedSaveStrategyTest {


  private DetailedSaveStrategy strategy;

  private MemoryFileSystem fs;

  private TestDeliveryManager deliveryManager;

  private User user;

  private ResourceOptions resourceOptions;

  private UserCPsettings settings;

  @Before
  public void init() throws AdminException {
    fs = new MemoryFileSystem();
    deliveryManager = new TestDeliveryManager();

    settings = prepareSettings();
    user = prepareUser();
    resourceOptions = new ResourceOptions(user, new File("workDir"), settings);
    resourceOptions.setSourceAddress(new Address("+79139489906"));
    resourceOptions.setEncoding("utf-8");
    _init();
  }

  private void _init() throws AdminException {
    ContentProviderContext context = new SingleUserContentPContext(prepareUser(), deliveryManager, fs);

    strategy = new DetailedSaveStrategy(context, new LocalResourceWithChecking(new File("dir"), fs) , resourceOptions);
  }


  @After
  public void shutdown() {
    if(deliveryManager != null) {
      deliveryManager.shutdown();
    }
  }

  @Test
  public void testSynchrDownload() throws AdminException {

    File resourceFile = prepareResourceFile(false, null);

    strategy.synchronize(true);
    assertTrue("File doesnt exist", fs.exists(resourceFile));
    assertTrue("File doesn't exist!", fs.exists(new File("workDir" + File.separatorChar + "detailedLocalCopy", "test.csv")));

  }

  @Test
  public void testSynchrInProgressNothing() throws AdminException {

    File resourceFile = prepareResourceFile(false, ".inprocess");
    File localFile = prepareLocalFile(false, ".inprocess");

    strategy.synchronize(true);

    assertTrue("File doesn't exist", fs.exists(resourceFile));
    assertTrue("File doesn't exist!", fs.exists(localFile));

  }

  @Test
  public void testSynchrInProgressRename() throws AdminException {

    File resourceFile = prepareResourceFile(false, null);
    File localFile = prepareLocalFile(false, ".inprocess");

    strategy.synchronize(true);

    assertFalse("File exist", fs.exists(resourceFile));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getAbsolutePath()+".inprocess")));
    assertTrue("File doesn't exist!", fs.exists(localFile));

  }

  @Test
  public void testSynchrInProgressRemove() throws AdminException {
    File dir = new File("dir");
    fs.mkdirs(dir);

    File localFile = prepareLocalFile(false, ".inprocess");

    strategy.synchronize(true);

    assertFalse("File exist", fs.exists(localFile));

  }

  @Test
  public void testSynchrFinished() throws AdminException {

    File resourceFile = prepareResourceFile(false, ".inprocess");
    File localFile = prepareLocalFile(false, ".finished");
    File report = createEmptyLocalFile(".rep");

    strategy.synchronize(true);

    assertTrue("File doesn't exist", fs.exists(localFile));
    assertTrue("File doesn't exist", fs.exists(report));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getParentFile(), "test.csv.finished")));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getParentFile(), "test.csv.rep")));
    assertFalse("File exists", fs.exists(resourceFile));
  }

  @Test
  public void testSynchrFinishedWOReport() throws AdminException {
    shutdown();


    fs = new MemoryFileSystem();
    deliveryManager = new TestDeliveryManager();
    user = prepareUser();

    settings = prepareSettings();
    settings.setCreateReports(false);
    resourceOptions = new ResourceOptions(user, new File("workDir"), settings);
    resourceOptions.setSourceAddress(new Address("+79139489906"));
    resourceOptions.setEncoding("utf-8");
    _init();


    File resourceFile = prepareResourceFile(false, ".inprocess");
    File localFile = prepareLocalFile(false, ".finished");

    strategy.synchronize(true);

    assertTrue("File doesn't exist", fs.exists(localFile));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getParentFile(), "test.csv.finished")));
    assertFalse("File exists", fs.exists(resourceFile));
  }

  @Test
  public void testSynchrFinishedRemove() throws AdminException {
    File dir = new File("dir");
    fs.mkdirs(dir);

    File localFile = prepareLocalFile(false, ".finished");
    File report = createEmptyLocalFile(".rep");

    strategy.synchronize(true);

    assertFalse("File exist", fs.exists(localFile));
    assertFalse("File exist", fs.exists(report));
  }

  @Test
  public void testSynchrFinishedNothing() throws AdminException {
    File resourceFile = prepareResourceFile(false, ".finished");
    File localFile = prepareLocalFile(false, ".finished");
    File report = createEmptyLocalFile(".rep");

    strategy.synchronize(true);

    assertTrue("File doesn't exist", fs.exists(localFile));
    assertTrue("File doesn't exist", fs.exists(report));
    assertTrue("File doesn't exist", fs.exists(resourceFile));
  }


  @Test
  public void testSynchrFinishedReportUploadError() throws AdminException{
    File resourceFile = prepareResourceFile(false, ".inprocess");
    File localFile = prepareLocalFile(false, ".finished");
    File report = createEmptyLocalFile(".rep");
    File reportPart = createEmptyResourceFile(".rep.part");

    strategy.synchronize(true);

    assertTrue("File doesn't exist", fs.exists(localFile));
    assertTrue("File doesn't exist", fs.exists(report));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getParentFile(), "test.csv.finished")));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getParentFile(), "test.csv.rep")));
    assertFalse("File exists", fs.exists(reportPart));
    assertFalse("File exists", fs.exists(resourceFile));
  }



  @Test
  public void testProcessCreateSingleText() throws AdminException {
    testProcessCreate(true);
  }

  @Test
  public void testProcessCreateMultiText() throws AdminException {
    testProcessCreate(false);
  }

  @Test
  public void testProcessNotCreate() throws AdminException {

    File resourceFile = prepareResourceFile(false, null);

    strategy.synchronize(false);

    assertTrue("File doesn't exist!", fs.exists(resourceFile));
    assertFalse("File exist!", fs.exists(new File("workDir" + File.separatorChar + "simpleLocalCopy", "test.csv")));

    strategy.process(false);

    assertTrue("File doesn't exist!", fs.exists(resourceFile));
    assertFalse("File exist!", fs.exists(new File("workDir" + File.separatorChar + "simpleLocalCopy", "test.csv")));
    assertFalse("File exist!", fs.exists(new File("workDir" + File.separatorChar + "simpleLocalCopy", "test.csv.inprocess")));

    assertEquals(deliveryManager.countDeliveries(user.getLogin(), user.getPassword(), new DeliveryFilter()), 0);
  }



  @Test
  public void testCreateInformerOffline() throws AdminException {
    shutdown();

    fs = new MemoryFileSystem();
    deliveryManager = new TestDeliveryManager(){
      @Override
      public Delivery createDeliveryWithIndividualTexts(String login, String password, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
        throw new DeliveryException(DeliveryException.ErrorStatus.ServiceOffline,"offline");
      }
    };
    user = prepareUser();

    settings = prepareSettings();
    settings.setCreateReports(false);
    resourceOptions = new ResourceOptions(user, new File("workDir"), settings);
    resourceOptions.setSourceAddress(new Address("+79139489906"));
    resourceOptions.setEncoding("utf-8");
    _init();


    File resourceFile = prepareResourceFile(false, null);

    strategy.process(true);

    assertTrue("File doesn't exist!", fs.exists(resourceFile));
    assertFalse("File exist!", fs.exists(new File("workDir" + File.separatorChar + "simpleLocalCopy", "test.csv")));
    assertFalse("File exist!", fs.exists(new File("workDir" + File.separatorChar + "simpleLocalCopy", "test.csv.inprocess")));

    deliveryManager.shutdown();

    deliveryManager = new TestDeliveryManager();

   _init();

    strategy.process(true);

    assertFalse("File exists!", fs.exists(resourceFile));
    assertFalse("File exists!", fs.exists(new File("workDir" + File.separatorChar + "detailedLocalCopy", "test.csv")));
    assertFalse("File exists!", fs.exists(resourceFile));
    assertFalse("File exist", fs.exists(resourceFile));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getAbsolutePath()+".inprocess")));
    assertTrue("File doesn't exist", fs.exists(new File("workDir" + File.separatorChar + "detailedLocalCopy", "test.csv.inprocess")));
  }



  @Test
  public void testCreateReportInformerOffline() throws AdminException, InterruptedException {
    shutdown();

    fs = new MemoryFileSystem();
    final boolean[] fail = new boolean[]{true};
    deliveryManager = new TestDeliveryManager(){
      @Override
      public void getMessages(String login, String password, MessageFilter filter, int _pieceSize, Visitor<Message> visitor) throws AdminException {
        if(fail[0]) {
          throw new DeliveryException(DeliveryException.ErrorStatus.ServiceOffline,"offline");
        }else {
          super.getMessages(login, password, filter, _pieceSize, visitor);
        }
      }
    };
    user = prepareUser();

    settings = prepareSettings();
    resourceOptions = new ResourceOptions(user, new File("workDir"), settings);
    resourceOptions.setSourceAddress(new Address("+79139489906"));
    resourceOptions.setEncoding("utf-8");
    _init();


    File resourceFile = prepareResourceFile(false, null);

    strategy.process(true);

    assertFalse("File exists!", fs.exists(resourceFile));
    assertFalse("File exists!", fs.exists(new File("workDir" + File.separatorChar + "detailedLocalCopy", "test.csv")));
    assertFalse("File exists!", fs.exists(resourceFile));
    assertFalse("File exist", fs.exists(resourceFile));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getAbsolutePath()+".inprocess")));
    assertTrue("File doesn't exist", fs.exists(new File("workDir" + File.separatorChar + "detailedLocalCopy", "test.csv.inprocess")));

    Thread.sleep(51);
    deliveryManager.forceModifyDeliveries();

    fail[0] = false;

    strategy.process(true);
    assertTrue("File doesn't exist", fs.exists(new File("workDir" + File.separatorChar + "detailedLocalCopy", "test.csv.finished")));
    assertTrue("File doesn't exist", fs.exists(new File("workDir" + File.separatorChar + "detailedLocalCopy", "test.csv.rep")));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getAbsolutePath() + ".finished")));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getAbsolutePath() + ".rep")));

  }



  @Test
  public void testProcessAll() throws Exception {

    File resourceFile = prepareResourceFile(false, null);

    strategy.process(true);

    Thread.sleep(51);
    deliveryManager.forceModifyDeliveries();


    strategy.process(true);

    assertTrue("File doesn't exist", fs.exists(new File("workDir" + File.separatorChar + "detailedLocalCopy", "test.csv.finished")));
    assertTrue("File doesn't exist", fs.exists(new File("workDir" + File.separatorChar + "detailedLocalCopy", "test.csv.rep")));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getAbsolutePath() + ".finished")));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getAbsolutePath() + ".rep")));

  }

   private void testProcessCreate(boolean isSingleText) throws AdminException {

    File resourceFile = prepareResourceFile(isSingleText, null);
    strategy.process(true);

    assertFalse("File exists!", fs.exists(resourceFile));
    assertFalse("File exists!", fs.exists(new File("workDir" + File.separatorChar + "detailedLocalCopy", "test.csv")));
    assertFalse("File exists!", fs.exists(resourceFile));
    assertFalse("File exist", fs.exists(resourceFile));
    assertTrue("File doesn't exist", fs.exists(new File(resourceFile.getAbsolutePath()+".inprocess")));
    assertTrue("File doesn't exist", fs.exists(new File("workDir" + File.separatorChar + "detailedLocalCopy", "test.csv.inprocess")));

    final Delivery[] exist = new Delivery[]{null};
    deliveryManager.getDeliveries(user.getLogin(), user.getPassword(), new DeliveryFilter(), 1000, new Visitor<Delivery>() {
      @Override
      public boolean visit(Delivery value) throws AdminException {
        exist[0] = value;
        return false;
      }
    });

    assertNotNull("Delivery doesn't exist", exist[0]);
    if(isSingleText) {
      assertNotNull(exist[0].getSingleText());
    }else {
      assertNull(exist[0].getSingleText());
    }
    assertNotNull("Delivery md5 is empty", exist[0].getProperty("cp-md5"));

    assertEquals(
        deliveryManager.countMessages(user.getLogin(), user.getPassword(), new MessageFilter(exist[0].getId(), new Date(0), new Date())),
        3
    );
  }


  private UserCPsettings prepareSettings() {
    UserCPsettings settings = new UserCPsettings();
    settings.setHost("host");
    settings.setWorkType(UserCPsettings.WorkType.detailed);
    settings.setSourceAddress(new Address("1312313131"));
    settings.setCreateReports(true);
    return settings;
  }


  private static User prepareUser() {
    User u = new User();
    u.setLogin("a");
    u.setPassword("");
    u.setAllRegionsAllowed(true);
    return u;
  }

  private File createEmptyLocalFile(String posfix) throws AdminException{
    File dir = new File("workDir", "detailedLocalCopy");
    fs.mkdirs(dir);
    File file = new File(dir, "test.csv"+(posfix == null ? "" : posfix));
    fs.createNewFile(file);
    return file;
  }
  private File createEmptyResourceFile(String posfix) throws AdminException{
    File dir = new File("dir");
    fs.mkdirs(dir);
    File file = new File(dir, "test.csv"+(posfix == null ? "" : posfix));
    fs.createNewFile(file);
    return file;
  }


  private File prepareResourceFile(boolean isSingleText, String posfix) throws AdminException {
    File dir = new File("dir");
    fs.mkdirs(dir);
    File file = new File(dir, "test.csv"+(posfix == null ? "" : posfix));
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(file, false))));
      prepareFile(writer, isSingleText);
    }finally{
      if(writer != null) {
        writer.close();
      }
    }
    assertTrue("File doesn't exist!", fs.exists(file));
    return file;
  }

  private File prepareLocalFile(boolean isSingleText, String posfix) throws AdminException {
    File dir = new File("workDir", "detailedLocalCopy");
    fs.mkdirs(dir);
    File file = new File(dir, "test.csv"+(posfix == null ? "" : posfix));
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(file, false))));
      prepareFile(writer, isSingleText);
    }finally{
      if(writer != null) {
        writer.close();
      }
    }
    assertTrue("File doesn't exist!", fs.exists(file));
    return file;
  }

  private void prepareFile(PrintWriter writer, boolean isSingleText) {
      writer.println("+79139489906|text" + (isSingleText ? "" : "1"));
      writer.println("89139489907|text");
      writer.println("7913948990wrasrafasfsafasasf|text");
      writer.println("79139489908|text");
  }

}
