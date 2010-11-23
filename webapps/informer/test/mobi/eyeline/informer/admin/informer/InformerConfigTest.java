package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.config.XmlConfig;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class InformerConfigTest {

  private static File configFile, backupDir;

  @BeforeClass
  public static void init() throws Exception {
    configFile = TestUtils.exportResourceToRandomFile(InformerConfigTest.class.getResourceAsStream("config.xml"), ".config");
    backupDir = TestUtils.createRandomDir(".config.backup");
  }

  @AfterClass
  public static void shutdown() {
    if (configFile != null) {
      configFile.delete();
    }
    if (backupDir != null) {
      TestUtils.recursiveDeleteFolder(backupDir);
    }
  }

  @Test
  public void loadTest() throws Exception {
    InformerSettings s = new InformerConfig().load(FileSystem.getFSForSingleInst().getInputStream(configFile));

    assertEquals("adminHost", s.getAdminHost());
    assertEquals(8080, s.getAdminPort());
    assertEquals("dcpHost", s.getDcpHost());
    assertEquals(8088, s.getDcpPort());
    assertEquals("pvssHost", s.getPersHost());
    assertEquals(25403, s.getPersPort());
    assertEquals("stat", s.getStatDir());
    assertEquals("store", s.getStoreDir());
  }

  @Test
  public void saveTest() throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(configFile);

    File tmpFile = new File(configFile.getAbsolutePath() + ".tmp");

    try {
      InformerSettings s = new InformerConfig().load(FileSystem.getFSForSingleInst().getInputStream(configFile));
      new InformerConfig().save(FileSystem.getFSForSingleInst().getInputStream(configFile), FileSystem.getFSForSingleInst().getOutputStream(tmpFile, true), s);

      XmlConfig config1 = new XmlConfig();
      config1.load(tmpFile);

      assertTrue(config.equals(config1));
    } finally {
      tmpFile.delete();
    }
  }
}
