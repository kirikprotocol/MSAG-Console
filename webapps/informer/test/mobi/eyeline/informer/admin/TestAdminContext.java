package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.blacklist.TestBlacklistManager;
import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import mobi.eyeline.informer.admin.informer.TestInformerConfigManager;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.regions.TestRegionsManager;
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
    TestUtils.exportResource(UsersManagerTest.class.getResourceAsStream("users.xml"), new File(config.getUsersFile()), false);
    TestUtils.exportResource(TestInformerConfigManager.class.getResourceAsStream("config.xml"), new File(baseDir, "conf"+File.separatorChar+"config.xml"), false);
    TestUtils.exportResource(TestSmscManager.class.getResourceAsStream("smsc.xml"), new File(baseDir, "conf"+File.separatorChar+"smsc.xml"), false);
    TestUtils.exportResource(TestRegionsManager.class.getResourceAsStream("regions.xml"), new File(baseDir, "conf"+File.separatorChar+"regions.xml"), false);
  }

  public TestAdminContext(File appBaseDir, WebConfig webConfig) throws InitException {
    fileSystem = new TestFileSystem();
    try {
      prepareServices(webConfig, appBaseDir);
      File usersFile = new File(webConfig.getUsersFile());
      usersManager = new TestUsersManager(usersFile, new File(usersFile.getParentFile(), "backup"), fileSystem);

      journal = new Journal(new File(webConfig.getJournalDir()), fileSystem);
      informerConfigManager = new TestInformerConfigManager(new File(appBaseDir, "conf"+File.separatorChar+"config.xml"),
          new File(appBaseDir, "conf"+File.separatorChar+"backup"), fileSystem);
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
    } catch (IOException e) {
      throw new InitException(e);
    }catch (AdminException e) {
      throw new InitException(e);
    }
  }

}
