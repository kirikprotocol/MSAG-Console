package ru.novosoft.smsc.admin.reschedule;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.*;

import static org.junit.Assert.*;
import static org.junit.Assert.assertEquals;

/**
 * @author Artem Snopkov
 */
public class RescheduleManagerTest {

  private static File configFile, backupDir;

  @BeforeClass
  public static void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(RescheduleManagerTest.class.getResourceAsStream("schedule.xml"), ".reschedule");
    backupDir = TestUtils.createRandomDir(".reschedule.backup");
  }

  @AfterClass
  public static void afterClass() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  @Test
  public void loadTest() throws AdminException {
    RescheduleManager manager = new RescheduleManager(configFile, backupDir, new TestClusterController(), FileSystem.getFSForSingleInst());

    assertFalse(manager.isChanged());

    assertEquals(20, manager.getScheduleLimit());
    assertEquals("30s,1m,5m,15m,30m,1h,6h,12h,1d:*", manager.getDefaultReschedule());

    Collection<Reschedule> reschedules = manager.getReschedules();
    assertNotNull(reschedules);
    assertEquals(2, reschedules.size());

    Iterator<Reschedule> iter = reschedules.iterator();

    assertEquals(new Reschedule("30s,11m,15m", 8), iter.next());
    assertEquals(new Reschedule("30s,1m,5m,15m,30m,1h,6h,12h,1d", 1028, 255, 20, 1027, 88, 100, 69), iter.next());
  }

  @Test
  public void saveTest() throws AdminException, XmlConfigException {
    // Загружаем первоначальный конфиг
    XmlConfig cfg = new XmlConfig();
    cfg.load(configFile);

    // Создаем инстанц SmscConfig
    TestClusterController clusterController = new TestClusterController();
    RescheduleManager config1 = new RescheduleManager(configFile, backupDir, clusterController, FileSystem.getFSForSingleInst());
    config1.setScheduleLimit(20);
    // Сохраняем SmscConfig
    config1.apply();

    assertFalse(config1.isChanged());
    assertTrue(clusterController.applyRescheduleCalled);

    // Проверяем, что в директории backup появились файлы
    assertFalse(backupDir.delete()); // Не можем удалить директорию т.к. там появились файлы

    // Снова загружаем конфиг
    XmlConfig cfg1 = new XmlConfig();
    cfg1.load(configFile);

    // Проверяем эквивалентность первоначальному конфигу.
    assertEquals(cfg, cfg1);
  }

  @Test
  public void resetTest() throws AdminException {
    RescheduleManager config = new RescheduleManager(configFile, backupDir, new TestClusterController(), FileSystem.getFSForSingleInst());

    assertFalse(config.isChanged());

    config.setScheduleLimit(100);
    config.setDefaultReschedule("10m,30s");
    config.setReschedules(new ArrayList<Reschedule>());

    assertTrue(config.isChanged());

    config.reset();

    assertEquals(20, config.getScheduleLimit());
    assertEquals("30s,1m,5m,15m,30m,1h,6h,12h,1d:*", config.getDefaultReschedule());

    Collection<Reschedule> reschedules = config.getReschedules();
    assertNotNull(reschedules);
    assertEquals(2, reschedules.size());

    Iterator<Reschedule> iter = reschedules.iterator();

    assertEquals(new Reschedule("30s,11m,15m", 8), iter.next());
    assertEquals(new Reschedule("30s,1m,5m,15m,30m,1h,6h,12h,1d", 1028, 255, 20, 1027, 88, 100, 69), iter.next());
  }

  @Test
  public void resetFailedTest() throws AdminException {
    RescheduleManager config = new RescheduleManager(configFile, backupDir, new TestClusterController(), FileSystem.getFSForSingleInst());

    assertFalse(config.isChanged());

    config.setScheduleLimit(100);
    config.setDefaultReschedule("10m,30s");
    config.setReschedules(new ArrayList<Reschedule>());

    assertTrue(config.isChanged());

    configFile.delete();

    try {
      config.reset();
      assertTrue(false);
    } catch (AdminException ignored) {
    }

    assertTrue(config.isChanged());

    assertEquals(100, config.getScheduleLimit());
    assertEquals("10m,30s", config.getDefaultReschedule());

    Collection<Reschedule> reschedules = config.getReschedules();
    assertNotNull(reschedules);
    assertEquals(0, reschedules.size());
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
   RescheduleManager manager = new RescheduleManager(configFile, backupDir, new ClusterControllerImpl(), FileSystem.getFSForSingleInst());

    Map<Integer, SmscConfigurationStatus> states = manager.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  public class ClusterControllerImpl extends TestClusterController {
    public ConfigState getMainConfigState() throws AdminException {
      long now = configFile.lastModified();
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 1);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }
}
