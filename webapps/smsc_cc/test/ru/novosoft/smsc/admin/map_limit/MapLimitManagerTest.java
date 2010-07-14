package ru.novosoft.smsc.admin.map_limit;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class MapLimitManagerTest {

  private File configFile, backupDir;

  @Before
  public void before() throws IOException {
    configFile = TestUtils.exportResourceToRandomFile(MapLimitManagerTest.class.getResourceAsStream("maplimits.xml"), ".maplimits");
    backupDir = TestUtils.createRandomDir(".fraudbackup");
  }

  @After
  public void after() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  private void validateConfig(MapLimitManager manager) {
    assertEquals(10000, manager.getDlgLimitIn());
    assertEquals(10000, manager.getDlgLimitInSri());
    assertEquals(500, manager.getDlgLimitInUssd());
    assertEquals(1000, manager.getDlgLimitOutSri());
    assertEquals(1000, manager.getDlgLimitUssd());

    int[] noSriCodes = manager.getUssdNoSriCodes();
    assertNotNull(noSriCodes);
    assertEquals(3, noSriCodes.length);
    assertArrayEquals(new int[]{12, 15, 18}, noSriCodes);

    CongestionLevel clevels[] = manager.getCongestionLevels();
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
  public void loadTest() throws AdminException {
    MapLimitManager manager = new MapLimitManager(configFile, backupDir, new TestClusterController(), FileSystem.getFSForSingleInst());

    assertFalse(manager.isChanged());

    validateConfig(manager);
  }

  @Test
  public void resetTest() throws AdminException {
    MapLimitManager manager = new MapLimitManager(configFile, backupDir, new TestClusterController(), FileSystem.getFSForSingleInst());

    assertFalse(manager.isChanged());

    manager.setDlgLimitIn(1231);
    manager.setDlgLimitInSri(321);

    assertTrue(manager.isChanged());

    manager.reset();

    assertFalse(manager.isChanged());

    validateConfig(manager);
  }

  @Test
  public void resetFailedTest() throws AdminException {
    MapLimitManager manager = new MapLimitManager(configFile, backupDir, new TestClusterController(), FileSystem.getFSForSingleInst());

    assertFalse(manager.isChanged());

    manager.setDlgLimitIn(1231);
    manager.setDlgLimitInSri(321);

    assertTrue(manager.isChanged());

    configFile.delete();

    try {
      manager.reset();
      assertFalse(true);
    } catch (AdminException e) {}

    assertTrue(manager.isChanged());

    assertEquals(1231, manager.getDlgLimitIn());
    assertEquals(321, manager.getDlgLimitInSri());
  }

  @Test
  public void applyTest() throws AdminException {

    TestClusterController controller = new TestClusterController();
    MapLimitManager manager = new MapLimitManager(configFile, backupDir, controller, FileSystem.getFSForSingleInst());

    assertFalse(manager.isChanged());

    manager.setDlgLimitIn(1231);
    manager.setDlgLimitInSri(321);

    assertTrue(manager.isChanged());

    manager.apply();

    assertFalse(manager.isChanged());
    assertTrue(controller.applyMapLimitsCalled);

    MapLimitManager manager1 = new MapLimitManager(configFile, backupDir, controller, FileSystem.getFSForSingleInst());

    assertEquals(1231, manager1.getDlgLimitIn());
    assertEquals(321, manager1.getDlgLimitInSri());
    assertEquals(500, manager1.getDlgLimitInUssd());
    assertEquals(1000, manager1.getDlgLimitOutSri());
    assertEquals(1000, manager1.getDlgLimitUssd());

    int[] noSriCodes = manager1.getUssdNoSriCodes();
    assertNotNull(noSriCodes);
    assertEquals(3, noSriCodes.length);
    assertArrayEquals(new int[]{12, 15, 18}, noSriCodes);

    CongestionLevel clevels[] = manager1.getCongestionLevels();
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
  public void setCongestionLevelsTest() throws AdminException {
    MapLimitManager manager = new MapLimitManager(configFile, backupDir, new TestClusterController(), FileSystem.getFSForSingleInst());
    try {
      CongestionLevel levels[] = new CongestionLevel[MapLimitManager.MAX_CONGESTON_LEVELS];
      manager.setCongestionLevels(levels);
      assertFalse(true);
    } catch (IllegalArgumentException e) {
    }

    try {
      CongestionLevel levels[] = new CongestionLevel[]{new CongestionLevel(1, 2, 3, 4)};
      manager.setCongestionLevels(levels);
      assertFalse(true);
    } catch (IllegalArgumentException e) {
    }

    try {
      CongestionLevel levels[] = new CongestionLevel[]{
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4)
      };
      manager.setCongestionLevels(levels);

    } catch (IllegalArgumentException e) {
      assertFalse(true);
    }
  }


}
