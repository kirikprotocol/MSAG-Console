package ru.novosoft.smsc.admin.reschedule;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.Iterator;

import static org.junit.Assert.*;

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
    RescheduleManager manager = new RescheduleManager(configFile, backupDir, null, FileSystem.getFSForSingleInst());

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
    RescheduleManager config1 = new RescheduleManager(configFile, backupDir, new ClusterControllerImpl(), FileSystem.getFSForSingleInst());
    // Сохраняем SmscConfig
    config1.apply();

    // Проверяем, что в директории backup появились файлы
    assertFalse(backupDir.delete()); // Не можем удалить директорию т.к. там появились файлы

    // Снова загружаем конфиг
    XmlConfig cfg1 = new XmlConfig();
    cfg1.load(configFile);

    // Проверяем эквивалентность первоначальному конфигу.
    assertEquals(cfg, cfg1);
  }

  @Test
  public void isChangedTest() throws AdminException {
    RescheduleManager config = new RescheduleManager(configFile, backupDir, new ClusterControllerImpl(), FileSystem.getFSForSingleInst());

    assertFalse(config.isChanged());

    config.setScheduleLimit(100);

    assertTrue(config.isChanged());

    config.reset();

    assertFalse(config.isChanged());

    config.setScheduleLimit(100);

    assertTrue(config.isChanged());

    config.apply();

    assertFalse(config.isChanged());
  }

  private class ClusterControllerImpl extends ClusterController {

    public File getRescheduleConfig() {
      return configFile;
    }

  }
}
