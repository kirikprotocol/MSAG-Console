package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import mobi.eyeline.informer.web.users.UsersManagerTest;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class TestAdminContext extends AdminContext {

  private void prepareServices(File usersFile) throws IOException, AdminException {

    TestUtils.exportResource(UsersManagerTest.class.getResourceAsStream("users.xml"), usersFile, false);

  }

  public TestAdminContext(File appBaseDir, InstallationType type, File[] appMirrorDirs) throws AdminException {
    this.appBaseDir = appBaseDir;
//    this.servicesDir = new File(appBaseDir, "services");
    this.instType = type;

//    if (!servicesDir.exists())
//      servicesDir.mkdirs();


    if (instType == InstallationType.SINGLE) {
//      serviceManager = new TestServiceManagerSingle(servicesDir, smscInstancesNumber);
    }else {
//      serviceManager = new TestServiceManagerHA(servicesDir, smscInstancesNumber, new String[] {"host0", "host1", "host2"});
    }
    fileSystem = new TestFileSystem();

  }
//
//  public TestAdminContext() throws AdminException {
//    this(new File("."), new File("test", "webconfig.xml"));
//  }



}
