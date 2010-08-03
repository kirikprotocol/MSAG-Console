package ru.novosoft.smsc.admin.cluster_controller;

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
public class ClusterControllerConfigTest {

  private File configFile;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(ClusterControllerConfigTest.class.getResourceAsStream("config.xml"), ".clustercontroller");
  }

  private ClusterControllerSettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new ClusterControllerConfig());
  }

  private void saveSettings(ClusterControllerSettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new ClusterControllerConfig(), s);
  }

  @After
  public void afterClass() {
    if (configFile != null)
      configFile.delete();
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
    c.load(configFile);

    ClusterControllerSettings s = loadSettings();
    saveSettings(s);

    XmlConfig c1 = new XmlConfig();
    c1.load(configFile);

    assertEquals(c, c1);
  }
}
