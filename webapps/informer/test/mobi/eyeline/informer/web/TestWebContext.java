package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.TestAdminContext;
import mobi.eyeline.informer.web.auth.AuthenticatorImpl;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.journal.Journal;
import mobi.eyeline.informer.web.journal.JournalFileDataSource;
import mobi.eyeline.informer.web.users.TestUsersManager;
import mobi.eyeline.informer.web.users.UsersManagerTest;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

/**
 * @author Aleksandr Khalitov
 */
public class TestWebContext extends WebContext{

  private void prepareServices(WebConfig config) throws IOException, AdminException {
    TestUtils.exportResource(UsersManagerTest.class.getResourceAsStream("users.xml"), new File(config.getUsersFile()), false);
  }

  protected void prepare(File baseDir) throws InitException {
    try {
      this.adminContext = new TestAdminContext(baseDir, webConfig.getInstallationType(), webConfig.getAppMirrorDirs());
      prepareServices(webConfig);
      File usersFile = new File(webConfig.getUsersFile());
      usersManager = new TestUsersManager(usersFile, new File(usersFile.getParentFile(), "backup"), adminContext.getFileSystem());
      this.authenticator = new AuthenticatorImpl(usersManager);
      journal = new Journal(new JournalFileDataSource(new File(webConfig.getJournalDir()), adminContext.getFileSystem()));       //todo file system
      configuration = new Configuration(journal, adminContext, usersManager);
    } catch (Exception e) {
      throw new InitException(e);
    }


  }


}
