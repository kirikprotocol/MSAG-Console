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
public class SimpleSaveStrategyTest {

  private SimpleSaveStrategy strategy;

  private MemoryFileSystem fs;

  private TestDeliveryManager deliveryManager;

  private User user;
  private UserCPsettings settings;

  @Before
  public void init() throws AdminException {
    fs = new MemoryFileSystem();
    deliveryManager = new TestDeliveryManager();
    _init();
  }

  private void _init() throws AdminException {
    user = prepareUser();
    settings = prepareSettings();

    ContentProviderContext context = new SingleUserContentPContext(prepareUser(), deliveryManager, fs);

    ResourceOptions resourceOptions = new ResourceOptions(user, new File("workDir"), settings);
    resourceOptions.setSourceAddress(new Address("+79139489906"));
    resourceOptions.setEncoding("utf-8");

    strategy = new SimpleSaveStrategy(context, new LocalResourceWithChecking(new File("dir"), fs) , resourceOptions);

  }


  @After
  public void shutdown() {
    if(deliveryManager != null) {
      deliveryManager.shutdown();
    }
  }

  private UserCPsettings prepareSettings() {
    UserCPsettings settings = new UserCPsettings();
    settings.setHost("host");
    settings.setWorkType(UserCPsettings.WorkType.simple);
    return settings;
  }


  private static User prepareUser() {
    User u = new User();
    u.setLogin("a");
    u.setPassword("");
    u.setAllRegionsAllowed(true);
    return u;
  }


  @Test
  public void testSynchronization() throws AdminException {
    File resourceFile = prepareResourceFile(false);

    strategy.synchronize(true);
    assertTrue("File exist!", !fs.exists(resourceFile));
    assertTrue("File doesn't exist!", fs.exists(new File("workDir" + File.separatorChar + "simpleLocalCopy", "test.csv")));
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

    File resourceFile = prepareResourceFile(false);

    strategy.synchronize(false);

    assertTrue("File doesn't exist!", fs.exists(resourceFile));
    assertFalse("File exist!", fs.exists(new File("workDir" + File.separatorChar + "simpleLocalCopy", "test.csv")));

    strategy.process(false);

    assertTrue("File doesn't exist!", fs.exists(resourceFile));
    assertFalse("File exist!", fs.exists(new File("workDir" + File.separatorChar + "simpleLocalCopy", "test.csv")));

    assertEquals(deliveryManager.countDeliveries(user.getLogin(), user.getPassword(), new DeliveryFilter()), 0);

  }



  @Test
  public void testInformerOffline() throws AdminException {
    shutdown();
    deliveryManager = new TestDeliveryManager() {
      @Override
      public Delivery createDeliveryWithIndividualTexts(String login, String password, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
        throw new DeliveryException(DeliveryException.ErrorStatus.ServiceOffline ,"message");
      }
    };
    File resourceFile;
    try{
      _init();


      resourceFile = prepareResourceFile(false);
      strategy.process(true);

      assertEquals(deliveryManager.countDeliveries(user.getLogin(), user.getPassword(), new DeliveryFilter()), 0);


      assertFalse("File exists!", fs.exists(resourceFile));
      assertTrue("File doesnt't exists!", fs.exists(new File("workDir" + File.separatorChar + "simpleLocalCopy", "test.csv")));

    }finally {
      if(deliveryManager != null) {
        deliveryManager.shutdown();
      }
    }

    deliveryManager = new TestDeliveryManager();
    _init();

    assertTrue("File doesn't exists", fs.exists(new File("workDir" + File.separatorChar + "simpleLocalCopy", "test.csv")));

    strategy.process(true);

    assertFalse("File exists!", fs.exists(resourceFile));
    assertFalse("File exists!", fs.exists(new File("workDir" + File.separatorChar + "simpleLocalCopy", "test.csv")));

    assertEquals(deliveryManager.countDeliveries(user.getLogin(), user.getPassword(), new DeliveryFilter()), 1);

  }










  private void testProcessCreate(boolean isSingleText) throws AdminException {

    File resourceFile = prepareResourceFile(isSingleText);
    strategy.process(true);

    assertFalse("File exists!", fs.exists(resourceFile));
    assertFalse("File exists!", fs.exists(new File("workDir" + File.separatorChar + "simpleLocalCopy", "test.csv")));

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

  private File prepareResourceFile(boolean isSingleText) throws AdminException {
    File dir = new File("dir");
    fs.mkdirs(dir);
    File file = new File(dir, "test.csv");
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(file, false))));
      writer.println("+79139489906|text" + (isSingleText ? "" : "1"));
      writer.println("89139489907|text");
      writer.println("7913948990wrasrafasfsafasasf|text");
      writer.println("79139489908|text");
    }finally{
      if(writer != null) {
        writer.close();
      }
    }
    assertTrue("File doesn't exist!", fs.exists(file));
    return file;
  }


}
