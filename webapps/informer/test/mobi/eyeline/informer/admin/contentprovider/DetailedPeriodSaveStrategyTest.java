package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.TestDeliveryManager;
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

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * author: Aleksandr Khalitov
 */
public class DetailedPeriodSaveStrategyTest {

  private MemoryFileSystem fs;

  private TestDeliveryManager deliveryManager;

  private User user;

  private ResourceOptions resourceOptions;

  private UserCPsettings settings;

  private FileResource resource;

  private ContentProviderContext context;

  private File localDir;

  private File resourceDir;

  @Before
  public void init() throws AdminException {
    fs = new MemoryFileSystem();
    deliveryManager = new TestDeliveryManager(new File(""), fs);

    settings = prepareSettings();
    user = prepareUser();
    resourceOptions = new ResourceOptions(user, new File("workDir"), settings, 60);
    resourceOptions.setSourceAddress(new Address("+79139489906"));
    resourceOptions.setEncoding("utf-8");
    resource = new LocalResourceStubWithChecking(new File("dir"), fs);
    context = new SingleUserContentPContextStub(prepareUser(), deliveryManager, fs);


    resourceDir = new File("dir");
    fs.mkdirs(resourceDir);

    localDir = new File("workDir", "detailedPeriodLocalCopy");
    fs.mkdirs(localDir);
  }

  public DetailedPeriodSaveStrategy createStrategy() throws AdminException {
    return new DetailedPeriodSaveStrategy(context, resource , resourceOptions);
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
    settings.setWorkType(UserCPsettings.WorkType.detailed);
    settings.setSourceAddress(new Address("1312313131"));
    settings.setCreateReports(true);
    settings.setReportTimeoutMin(0);
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
    File file = new File(localDir, "test.csv"+(posfix == null ? "" : posfix));
    fs.createNewFile(file);
    return file;
  }


  private File createEmptyResourceFile(String posfix) throws AdminException{
    File file = new File(resourceDir, "test.csv"+(posfix == null ? "" : posfix));
    fs.createNewFile(file);
    return file;
  }


  private File prepareResourceFile(boolean isSingleText, String posfix) throws AdminException {
    File file = new File(resourceDir, "test.csv"+(posfix == null ? "" : posfix));
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
    File file = new File(localDir, "test.csv"+(posfix == null ? "" : posfix));
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



  @Test
  public void testCleanOnStart() throws AdminException {
    File file = createEmptyLocalFile(".tmp");
    assertTrue(fs.exists(file));
    createStrategy();
    assertFalse(fs.exists(file));
  }

  @Test
  public void testCreate() throws Exception {
    File file = prepareResourceFile(false, null);
    assertTrue(fs.exists(file));
    DetailedPeriodSaveStrategy strategy = createStrategy();
    strategy.process(true);
    assertFalse(fs.exists(file));
    assertTrue(fs.exists(new File(file.getParent(), file.getName()+".active")));

    assertTrue(fs.exists(new File(localDir, file.getName()+".active")));

    boolean report = false;
    boolean reportInfo = false;

    for(File f : fs.listFiles(localDir)) {
      String name = f.getName();
      if(name.contains("report")) {
        report = true;
      }else if(name.contains("period")){
        reportInfo = true;
      }
    }

    assertTrue(report);
    assertTrue(reportInfo);

    report = false;

    for(File f : fs.listFiles(resourceDir)) {
      String name = f.getName();
      if(name.contains("report")) {
        report = true;
        break;
      }
    }

    assertTrue(report);
  }









}
