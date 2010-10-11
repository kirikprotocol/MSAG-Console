package mobi.eyeline.informer.admin.util.config;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class ConfigHelperTest {

  private static File configFile, backupDir;

  @BeforeClass
  public static void init() throws Exception{
    configFile = TestUtils.exportResourceToRandomFile(ConfigHelperTest.class.getResourceAsStream("helper_test.xml"), ".helper_test");
    backupDir = TestUtils.createRandomDir(".config.backup");
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

  @Test
  public void backupRollback() throws Exception{
    FileSystem fs = FileSystem.getFSForSingleInst();
    File backup = ConfigHelper.createBackup(configFile, backupDir, fs);
    assertTrue(backup.exists());
    assertEquals(backup.length(), configFile.length());
    assertTrue(configFile.delete() && configFile.createNewFile());
    ConfigHelper.rollbackConfig(backup, configFile, fs);
    assertEquals(configFile.length(), backup.length());
  }
}
