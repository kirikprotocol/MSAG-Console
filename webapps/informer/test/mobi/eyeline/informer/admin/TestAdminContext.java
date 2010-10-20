package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.blacklist.TestBlacklistManager;
import mobi.eyeline.informer.admin.delivery.TestDeliveryManager;
import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import mobi.eyeline.informer.admin.informer.TestInformerManager;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.regions.TestRegionsManager;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicy;
import mobi.eyeline.informer.admin.retry_policies.TestRetryPolicyManager;
import mobi.eyeline.informer.admin.service.TestServiceManagerHA;
import mobi.eyeline.informer.admin.service.TestServiceManagerSingle;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.smsc.TestSmscManager;
import mobi.eyeline.informer.admin.users.TestUsersManager;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Тестовый AdminContext
 * @author Aleksandr Khalitov
 */
public class TestAdminContext extends AdminContext {

  private void prepareServices(File confDir) throws IOException, AdminException {
    TestUtils.exportResource(TestUsersManager.class.getResourceAsStream("users.xml"), new File(confDir,"users.xml"), false);
    TestUtils.exportResource(TestInformerManager.class.getResourceAsStream("config.xml"), new File(confDir, "config.xml"), false);
    TestUtils.exportResource(TestSmscManager.class.getResourceAsStream("smsc.xml"), new File(confDir, "smsc.xml"), false);
    TestUtils.exportResource(TestRegionsManager.class.getResourceAsStream("regions.xml"), new File(confDir, "regions.xml"), false);
    TestUtils.exportResource(TestRetryPolicyManager.class.getResourceAsStream("policies.xml"), new File(confDir, "policies.xml"), false);
  }

  public TestAdminContext(File appBaseDir, WebConfig webConfig) throws InitException {
    fileSystem = new TestFileSystem();
    File servicesDir = new File(appBaseDir, "services");
    File confDir = new File(servicesDir, "Informer"+File.separatorChar+"conf");
    servicesDir.mkdirs();
    confDir.mkdirs();
    try {
      prepareServices(confDir);



      if (webConfig.getInstallationType() == InstallationType.SINGLE)  {
        serviceManager = new TestServiceManagerSingle(servicesDir);
      }else {
        serviceManager = new TestServiceManagerHA(servicesDir, new String[] {"host0", "host1", "host2"});
      }

      journal = new Journal(new File(webConfig.getJournalDir()), fileSystem);
      informerManager = new TestInformerManager(new File(confDir, "config.xml"),
          new File(confDir, "backup"), fileSystem, serviceManager);
      infosme = new TestInfosme();
      usersManager = new TestUsersManager(infosme, new File(confDir, "users.xml"),new File(confDir, "backup"), fileSystem);
      
      blacklistManager = new TestBlacklistManager();
      smscManager = new TestSmscManager(infosme, new File(confDir, "smsc.xml"),
          new File(confDir, "backup"), fileSystem);
      for(Smsc s : smscManager.getSmscs()) {
        infosme.addSmsc(s.getName());
      }
      regionsManager = new TestRegionsManager(infosme, new File(confDir, "regions.xml"),
          new File(confDir, "backup"), fileSystem);
      for(Region s : regionsManager.getRegions()) {
        infosme.addRegion(s.getRegionId());
      }

      deliveryManager = new TestDeliveryManager();

      retryPolicyManager = new TestRetryPolicyManager(infosme, new File(confDir, "policies.xml"),
          new File(confDir, "backup"), fileSystem);

      for(RetryPolicy rp : retryPolicyManager.getRetryPolicies()) {
        infosme.addRetryPolicy(rp.getPolicyId());
      }

    } catch (IOException e) {
      throw new InitException(e);
    }catch (AdminException e) {
      throw new InitException(e);
    }
  }

  private static final List<Daemon> testDaemons = new ArrayList<Daemon>(3) {{
    for(int i =0; i<3; i++) {
      add(new TestDaemon(i));
    }
  }};

  @Override
  public List<Daemon> getDaemons() {
    List<Daemon> ds = super.getDaemons();
    ds.addAll(testDaemons);
    return ds;
  }


  private static class TestDaemon implements Daemon {

    private boolean isStarted = false;

    private String name;

    private TestDaemon(int index ) {
      name = "TestDaemon"+index;
    }

    public String getName() {
      return name;
    }

    public void start() throws AdminException {
      isStarted = true;
    }

    public void stop() throws AdminException {
      isStarted = false;
    }

    public boolean isStarted() {
      return isStarted;
    }
  }
}