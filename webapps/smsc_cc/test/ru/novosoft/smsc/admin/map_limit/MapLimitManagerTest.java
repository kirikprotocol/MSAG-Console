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
public class MapLimitManagerTest {

  private File configFile, backupDir;

  @Before
  public void before() throws IOException {
    configFile = TestUtils.exportResourceToRandomFile(MapLimitManagerTest.class.getResourceAsStream("maplimits.xml"), ".maplimits");
    backupDir = TestUtils.createRandomDir(".maplimitsbackup");
  }

  @After
  public void after() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  private MapLimitManager getManager(ClusterController cc) throws AdminException {
    MapLimitManager m = new MapLimitManager(configFile, backupDir, cc, FileSystem.getFSForSingleInst());
    return m;
  }

  private void validateConfig(MapLimitSettings settings) throws AdminException {
    assertEquals(10000, settings.getDlgLimitIn());
    assertEquals(10000, settings.getDlgLimitInSri());
    assertEquals(500, settings.getDlgLimitInUssd());
    assertEquals(1000, settings.getDlgLimitOutSri());
    assertEquals(1000, settings.getDlgLimitUssd());

    int[] noSriCodes = settings.getUssdNoSriCodes();
    assertNotNull(noSriCodes);
    assertEquals(3, noSriCodes.length);
    assertArrayEquals(new int[]{12, 15, 18}, noSriCodes);

    CongestionLevel clevels[] = settings.getCongestionLevels();
    assertNotNull(clevels);
    assertEquals(MapLimitManager.MAX_CONGESTON_LEVELS, clevels.length);
    assertEquals(new CongestionLevel(4000, 800, 1000, 100), clevels[0]);
    assertEquals(new CongestionLevel(3000, 1800, 2000, 100), clevels[1]);
    assertEquals(new CongestionLevel(2000, 2800, 3000, 100), clevels[2]);
    assertEquals(new CongestionLevel(1500, 3800, 4000, 100), clevels[3]);
    assertEquals(new CongestionLevel(1000, 4800, 5000, 100), clevels[4]);
    assertEquals(new CongestionLevel(800, 5800, 6000, 100), clevels[5]);
    assertEquals(new CongestionLevel(600, 6800, 7000, 100), clevels[6]);
    assertEquals(new CongestionLevel(400, 7800, 8000, 80), clevels[7]);
  }

  @Test
  public void getSettingsTest() throws AdminException {
    MapLimitManager manager = getManager(new TestClusterControllerStub());
    validateConfig(manager.getSettings());
  }

  @Test
  public void updateSettingsTest() throws AdminException {

    TestClusterControllerStub controller = new TestClusterControllerStub();
    MapLimitManager manager = getManager(controller);
    MapLimitSettings s = manager.getSettings();

    s.setDlgLimitIn(1231);
    s.setDlgLimitInSri(321);

    manager.updateSettings(s);

    MapLimitManager manager1 = getManager(controller);

    MapLimitSettings s1 = manager1.getSettings();

    assertEquals(1231, s1.getDlgLimitIn());
    assertEquals(321, s1.getDlgLimitInSri());
    assertEquals(500, s1.getDlgLimitInUssd());
    assertEquals(1000, s1.getDlgLimitOutSri());
    assertEquals(1000, s1.getDlgLimitUssd());

    int[] noSriCodes = s1.getUssdNoSriCodes();
    assertNotNull(noSriCodes);
    assertEquals(3, noSriCodes.length);
    assertArrayEquals(new int[]{12, 15, 18}, noSriCodes);

    CongestionLevel clevels[] = s1.getCongestionLevels();
    assertNotNull(clevels);
    assertEquals(MapLimitManager.MAX_CONGESTON_LEVELS, clevels.length);
    assertEquals(new CongestionLevel(4000, 800, 1000, 100), clevels[0]);
    assertEquals(new CongestionLevel(3000, 1800, 2000, 100), clevels[1]);
    assertEquals(new CongestionLevel(2000, 2800, 3000, 100), clevels[2]);
    assertEquals(new CongestionLevel(1500, 3800, 4000, 100), clevels[3]);
    assertEquals(new CongestionLevel(1000, 4800, 5000, 100), clevels[4]);
    assertEquals(new CongestionLevel(800, 5800, 6000, 100), clevels[5]);
    assertEquals(new CongestionLevel(600, 6800, 7000, 100), clevels[6]);
    assertEquals(new CongestionLevel(400, 7800, 8000, 80), clevels[7]);
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    MapLimitManager manager = getManager(new ClusterControllerImpl());

    Map<Integer, SmscConfigurationStatus> states = manager.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
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

}
