package ru.novosoft.smsc.admin.sme;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerStub;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class SmeManagerTest {

  private File configFile, backupDir;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(SmeManagerTest.class.getResourceAsStream("sme.xml"), ".sme");
    backupDir = TestUtils.createRandomDir(".sme.backup");
  }

  @After
  public void afterClass() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  public SmeManager getManager(ClusterController cc) throws AdminException {
    return new SmeManager(configFile, backupDir, cc, null, FileSystem.getFSForSingleInst());
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    SmeManager manager = getManager(new ClusterControllerImpl(true));

    Map<Integer, SmscConfigurationStatus> states = manager.getStatusForSmscs();
    assertNotNull(states);

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));

    assertNull(getManager(new ClusterControllerImpl(false)).getStatusForSmscs());
  }

  @Test
  public void nullGetStatusForSmscs() throws AdminException {
    SmeManager manager = getManager(new ClusterControllerImpl1());

    Map<Integer, SmscConfigurationStatus> states = manager.getStatusForSmscs();

    assertTrue(states.isEmpty());
  }



  public class ClusterControllerImpl extends TestClusterControllerStub {
    private boolean online;

    public ClusterControllerImpl(boolean online) {
      this.online = online;
    }

    public boolean isOnline() {
      return online;
    }

    public ConfigState getSmeConfigState() throws AdminException {
      long now = configFile.lastModified();
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 100);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }

  public class ClusterControllerImpl1 extends TestClusterControllerStub {
    public ConfigState getSmeConfigState() throws AdminException {
      return null;
    }
  }
}
