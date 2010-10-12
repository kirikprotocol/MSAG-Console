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

  private static InformerManager informerManager;

  @BeforeClass
  public static void init() throws Exception {
    configFile = TestUtils.exportResourceToRandomFile(InformerConfigManagerTest.class.getResourceAsStream("config.xml"), ".config");
    backupDir = TestUtils.createRandomDir(".config.backup");
    informerManager = new InformerManagerImpl(configFile, backupDir, FileSystem.getFSForSingleInst(), null);
  }

  @Test
  public void loadSave() throws Exception{
    InformerSettings oldS = new InformerSettings();
    oldS.setHost("host");
    oldS.setAdminPort(1212);
    oldS.setDeliveriesPort(1213);
    oldS.setPersHost("host1");
    oldS.setPersPort(1231);

    informerManager.updateSettings(oldS);
    InformerSettings newS = informerManager.getConfigSettings();
    assertEquals(oldS.getAdminPort(), newS.getAdminPort());
    assertEquals(oldS.getHost(), newS.getHost());
    assertEquals(oldS.getDeliveriesPort(), newS.getDeliveriesPort());
    assertEquals(oldS.getPersHost(), newS.getPersHost());
    assertEquals(oldS.getPersPort(), newS.getPersPort());
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
