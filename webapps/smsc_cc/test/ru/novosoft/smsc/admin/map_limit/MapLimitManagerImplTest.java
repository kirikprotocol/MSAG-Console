package ru.novosoft.smsc.admin.map_limit;

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
public class MapLimitManagerImplTest {

  private File configFile, backupDir;

  @Before
  public void before() throws IOException {
    configFile = TestUtils.exportResourceToRandomFile(MapLimitManagerImplTest.class.getResourceAsStream("maplimits.xml"), ".maplimits");
    backupDir = TestUtils.createRandomDir(".maplimitsbackup");
  }

  @After
  public void after() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  private MapLimitManagerImpl getManager(ClusterController cc) throws AdminException {
    MapLimitManagerImpl m = new MapLimitManagerImpl(configFile, backupDir, cc, FileSystem.getFSForSingleInst());
    return m;
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    MapLimitManagerImpl manager = getManager(new ClusterControllerImpl());

    Map<Integer, SmscConfigurationStatus> states = manager.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  @Test
  public void nullGetStatusForSmscs() throws AdminException {
    MapLimitManagerImpl manager = getManager(new ClusterControllerImpl1());

    Map<Integer, SmscConfigurationStatus> states = manager.getStatusForSmscs();

    assertTrue(states.isEmpty());
  }

  public class ClusterControllerImpl extends TestClusterControllerStub {
    public ConfigState getMapLimitConfigState() throws AdminException {
      long now = configFile.lastModified();
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 1);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }

  public class ClusterControllerImpl1 extends TestClusterControllerStub {
    public ConfigState getMapLimitConfigState() throws AdminException {
      return null;
    }
  }

}
