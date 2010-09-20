package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import mobi.eyeline.informer.admin.users.TestUsersManager;
import mobi.eyeline.informer.admin.users.UsersManagerTest;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class TestAdminContext extends AdminContext {

  private void prepareServices(AdminContextConfig cfg) throws IOException, AdminException {

    File usersFile = new File(cfg.getUsersFile());
    TestUtils.exportResource(UsersManagerTest.class.getResourceAsStream("users.xml"), usersFile, false);

  }

  public TestAdminContext(File appBaseDir, File initFile) throws AdminException {
    AdminContextConfig cfg = new AdminContextConfig(initFile);
    this.appBaseDir = appBaseDir;
//    this.servicesDir = new File(appBaseDir, "services");
    this.instType = cfg.getInstallationType();

//    if (!servicesDir.exists())
//      servicesDir.mkdirs();

    try {
      prepareServices(cfg);
    } catch (IOException e) {
      throw new AdminContextException("Can't create services dir!");
    }


    if (instType == InstallationType.SINGLE) {
//      serviceManager = new TestServiceManagerSingle(servicesDir, smscInstancesNumber);
    }else {
//      serviceManager = new TestServiceManagerHA(servicesDir, smscInstancesNumber, new String[] {"host0", "host1", "host2"});
    }
    fileSystem = new TestFileSystem();

    File usersFile = new File(cfg.getUsersFile());

    usersManager = new TestUsersManager(usersFile, new File(usersFile.getParentFile(), "backup"), fileSystem);

  }

  public TestAdminContext() throws AdminException {
    this(new File("."), new File("test", "webconfig.xml"));
  }



}
