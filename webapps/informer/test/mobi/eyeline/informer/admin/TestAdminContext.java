package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.blacklist.TestBlacklistManager;
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
import mobi.eyeline.informer.admin.users.UsersManagerTest;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class TestAdminContext extends AdminContext {

  private void prepareServices(WebConfig config, File baseDir) throws IOException, AdminException {
    TestUtils.exportResource(TestUsersManager.class.getResourceAsStream("users.xml"), new File(config.getUsersFile()), false);
    TestUtils.exportResource(TestInformerManager.class.getResourceAsStream("config.xml"), new File(baseDir, "conf"+File.separatorChar+"config.xml"), false);
    TestUtils.exportResource(TestSmscManager.class.getResourceAsStream("smsc.xml"), new File(baseDir, "conf"+File.separatorChar+"smsc.xml"), false);
    TestUtils.exportResource(TestRegionsManager.class.getResourceAsStream("regions.xml"), new File(baseDir, "conf"+File.separatorChar+"regions.xml"), false);
    TestUtils.exportResource(TestRetryPolicyManager.class.getResourceAsStream("policies.xml"), new File(baseDir, "conf"+File.separatorChar+"policies.xml"), false);    
  }

  public TestAdminContext(File appBaseDir, WebConfig webConfig) throws InitException {
    fileSystem = new TestFileSystem();
    File servicesDir = new File(appBaseDir, "conf");
    try {
      prepareServices(webConfig, appBaseDir);
      File usersFile = new File(webConfig.getUsersFile());
      usersManager = new TestUsersManager(usersFile, new File(usersFile.getParentFile(), "backup"), fileSystem);


      if (webConfig.getInstallationType() == InstallationType.SINGLE)  {
        serviceManager = new TestServiceManagerSingle(servicesDir);
      }else {
        serviceManager = new TestServiceManagerHA(servicesDir, new String[] {"host0", "host1", "host2"});
      }
      
      journal = new Journal(new File(webConfig.getJournalDir()), fileSystem);
      informerManager = new TestInformerManager(new File(appBaseDir, "conf"+File.separatorChar+"config.xml"),
          new File(appBaseDir, "conf"+File.separatorChar+"backup"), fileSystem, serviceManager);
      infosme = new TestInfosme();
      blacklistManager = new TestBlacklistManager();
      smscManager = new TestSmscManager(infosme, new File(appBaseDir, "conf"+File.separatorChar+"smsc.xml"),
          new File(appBaseDir, "conf"+File.separatorChar+"backup"), fileSystem);
      for(Smsc s : smscManager.getSmscs()) {
        infosme.addSmsc(s.getName());
      }
      regionsManager = new TestRegionsManager(infosme, new File(appBaseDir, "conf"+File.separatorChar+"regions.xml"),
          new File(appBaseDir, "conf"+File.separatorChar+"backup"), fileSystem);
      for(Region s : regionsManager.getRegions()) {
        infosme.addRegion(s.getRegionId());
      }
      
      retryPolicyManager = new TestRetryPolicyManager(infosme, new File(appBaseDir, "conf"+File.separatorChar+"policies.xml"),
          new File(appBaseDir, "conf"+File.separatorChar+"backup"), fileSystem);

      for(RetryPolicy rp : retryPolicyManager.getRetryPolicies()) {
        infosme.addRetryPolicy(rp.getPolicyId());
      }
    } catch (IOException e) {
      throw new InitException(e);
    }catch (AdminException e) {
      throw new InitException(e);
    }
  }

}