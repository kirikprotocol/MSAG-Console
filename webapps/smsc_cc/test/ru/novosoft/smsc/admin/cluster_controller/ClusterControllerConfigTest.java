package ru.novosoft.smsc.admin.cluster_controller;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigHelper;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
import ru.novosoft.smsc.util.config.XmlConfig;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.assertEquals;

/**
 * @author Artem Snopkov
 */
public class ClusterControllerConfigTest {

  private File configFile;
  private final MemoryFileSystem fs = new MemoryFileSystem();

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = fs.createNewFile("config.xml", ClusterControllerConfigTest.class.getResourceAsStream("config.xml"));
  }

  private ClusterControllerSettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new ClusterControllerConfig(), fs);
  }

  private void saveSettings(ClusterControllerSettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new ClusterControllerConfig(), s, fs);
  }

  @Test
  public void testSave() throws Exception {
    int oldListenerPort;
    {
      ClusterControllerSettings s = loadSettings();
      oldListenerPort = s.getListenerPort();
      s.setListenerPort(oldListenerPort + 1);
      saveSettings(s);
    }

    {
      ClusterControllerSettings s = loadSettings();
      assertEquals(oldListenerPort + 1, s.getListenerPort());
    }
  }

  @Test
  public void testLoad() throws Exception {
    ClusterControllerSettings m = loadSettings();
    assertEquals(9998, m.getListenerPort());
  }

  @Test
  public void testSaveUnusedParams() throws Exception {
    XmlConfig c = new XmlConfig();
    c.load(fs.getInputStream(configFile));

    ClusterControllerSettings s = loadSettings();
    saveSettings(s);

    XmlConfig c1 = new XmlConfig();
    c1.load(fs.getInputStream(configFile));

    assertEquals(c, c1);
  }
}
