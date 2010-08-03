package ru.novosoft.smsc.admin.archive_daemon;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigHelper;
import ru.novosoft.smsc.util.config.XmlConfig;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.assertEquals;

/**
 * @author Artem Snopkov
 */
public class ArchiveDaemonConfigTest {

  private File configFile;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(ArchiveDaemonConfigTest.class.getResourceAsStream("config.xml"), ".archivedaemon");
  }

  @After
  public void afterClass() {
    if (configFile != null)
      configFile.delete();
  }

  private ArchiveDaemonSettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new ArchiveDaemonConfig());
  }

  private void saveSettings(ArchiveDaemonSettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new ArchiveDaemonConfig(), s);
  }

  @Test
  public void testLoad() throws Exception {
    ArchiveDaemonSettings c = loadSettings();

    assertEquals(4096000, c.getIndexatorMaxFlushSpeed());
    assertEquals(Integer.valueOf(8), c.getIndexatorSmeAddrChunkSizes().get("MAP_PROXY"));
    assertEquals(30, c.getInterval());
    assertEquals("@smsc.arc.dir@", c.getLocationsBaseDestination());
    assertEquals("@smsc.arctxt.dir@", c.getLocationsTextDestinations());
    assertEquals("@smsc.arcnew.dir@", c.getLocationsSources().get("src1"));
    assertEquals(1, c.getQueriesInit());
    assertEquals(5, c.getQueriesMax());
    assertEquals(20000, c.getTransactionsMaxSmsCount());
    assertEquals(60, c.getTransactionsMaxTimeInterval());
    assertEquals("@listen.host@", c.getViewHost());
    assertEquals(1212, c.getViewPort());
    assertEquals(10, c.getViewTimeout());
  }

  @Test
  public void testSave() throws Exception {

    int oldViewPort;
    {
      ArchiveDaemonSettings s = loadSettings();
      oldViewPort = s.getViewPort();
      s.setViewPort(oldViewPort + 1);
      saveSettings(s);
    }

    {
      ArchiveDaemonSettings s = loadSettings();
      assertEquals(oldViewPort + 1, s.getViewPort());
    }
  }

  @Test
  public void testCopyUnusedParams() throws Exception {
    XmlConfig cfg = new XmlConfig();
    cfg.load(configFile);

    ArchiveDaemonSettings s = loadSettings();
    saveSettings(s);

    XmlConfig cfg1 = new XmlConfig();
    cfg1.load(configFile);

    assertEquals(cfg, cfg1);
  }
}
