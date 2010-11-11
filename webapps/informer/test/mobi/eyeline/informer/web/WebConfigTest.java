package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import static org.junit.Assert.assertEquals;

/**
 * @author Aleksandr Khalitov
 */
public class WebConfigTest {

  private static File configFile,backupDir ;
  private static FileSystem fileSys;

  @BeforeClass
  public static void init() throws Exception{
    configFile = TestUtils.exportResourceToRandomFile(WebConfigTest.class.getResourceAsStream("webconfig.xml"), ".webconfig");
    backupDir = TestUtils.createRandomDir(".webconfig.backup");
    fileSys = FileSystem.getFSForSingleInst();
  }

  @AfterClass
  public static void shutdown() {
    if(configFile!= null) {
      configFile.delete();
    }
    if(backupDir != null) {
      TestUtils.recursiveDeleteFolder(backupDir);
    }
  }


  @Test(expected = InitException.class)
  public void testNull() throws InitException, AdminException {  
    new WebConfigManager(null,backupDir,fileSys);
  }

  @Test
  public void testData() throws Exception{
    WebConfigManager config = new WebConfigManager(configFile,backupDir,fileSys);
    Assert.assertEquals(config.getInstallationType(), InstallationType.HA);
    assertEquals(config.getHSDaemonHost(), "localhost");
    assertEquals(config.getSingleDaemonHost(), "localhost");
    assertEquals(config.getHSDaemonPort(), 10000);
    assertEquals(config.getSingleDaemonPort(), 10000);
    assertEquals(config.getAppMirrorDirs().length, 1);
    assertEquals(config.getAppMirrorDirs()[0].getAbsoluteFile(), new File("mirror").getAbsoluteFile());
    assertEquals(config.getHSDaemonHosts().size(), 1);
    assertEquals(config.getHSDaemonHosts().iterator().next(),"sunfire");
    assertEquals(config.getJournalDir(), "journal");
  }

}
