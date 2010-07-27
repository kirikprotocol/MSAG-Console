package ru.novosoft.smsc.admin.cluster_controller;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class ClusterControllerManagerTest {

  private File configFile, backupDir;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(ClusterControllerManagerTest.class.getResourceAsStream("config.xml"), ".clustercontroller");
    backupDir = TestUtils.createRandomDir(".clustercontrollerbackup");
  }

  @After
  public void afterClass() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  private ClusterControllerManager getManager() throws AdminException {
    ClusterControllerManager m = new ClusterControllerManager(configFile, backupDir, FileSystem.getFSForSingleInst());
    m.reset();
    return m;
  }

  @Test
  public void loadTest() throws AdminException {
    ClusterControllerManager m = getManager();

    assertEquals(9998, m.getListenerPort());
  }

  @Test
  public void saveTest() throws XmlConfigException, AdminException {
    XmlConfig c = new XmlConfig();
    c.load(configFile);

    ClusterControllerManager m = getManager();
    m.setListenerPort(9998);

    assertTrue(m.isChanged());

    m.apply();

    assertFalse(m.isChanged());

    XmlConfig c1 = new XmlConfig();
    c1.load(configFile);

    assertEquals(c, c1);
  }

  @Test
  public void resetTest() throws AdminException {
    ClusterControllerManager m = getManager();
    m.setListenerPort(9999);

    assertTrue(m.isChanged());

    m.reset();

    assertFalse(m.isChanged());

    assertEquals(9998, m.getListenerPort());
  }
}
