package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;

import static org.junit.Assert.assertEquals;

/**
 * @author Aleksandr Khalitov
 */
public class InformerConfigManagerTest {

  private static File configFile, backupDir;

  private static InformerConfigManager informerConfigManager;

  @BeforeClass
  public static void init() throws Exception {
    configFile = TestUtils.exportResourceToRandomFile(InformerConfigManagerTest.class.getResourceAsStream("config.xml"), ".config");
    backupDir = TestUtils.createRandomDir(".config.backup");
    informerConfigManager = new InformerConfigManager(configFile, backupDir, FileSystem.getFSForSingleInst());
  }

  @Test
  public void loadSave() throws Exception{
    InformerSettings oldS = new InformerSettings();
    oldS.setHost("host");
    oldS.setAdminPort(1212);
    oldS.setDeliveriesPort(1213);
    informerConfigManager.updateSettings(oldS);
    InformerSettings newS = informerConfigManager.getConfigSettings();
    assertEquals(oldS.getAdminPort(), newS.getAdminPort());
    assertEquals(oldS.getHost(), newS.getHost());
    assertEquals(oldS.getDeliveriesPort(), newS.getDeliveriesPort());
  }



  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  @AfterClass
  public static void shutdown() {
    if(configFile != null) {
      configFile.delete();
    }
    if(backupDir != null) {
      TestUtils.recursiveDeleteFolder(backupDir);
    }

  }  
}
