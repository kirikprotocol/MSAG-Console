package ru.novosoft.smsc.admin.reschedule;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerStub;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class RescheduleManagerTest {

  private File configFile, backupDir;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(RescheduleManagerTest.class.getResourceAsStream("schedule.xml"), ".reschedule");
    backupDir = TestUtils.createRandomDir(".reschedule.backup");
  }

  @After
  public void afterClass() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  public RescheduleManager getManager(ClusterController cc) throws AdminException {
    RescheduleManager m = new RescheduleManager(configFile, backupDir, cc, FileSystem.getFSForSingleInst());
    return m;
  }

  @Test
  public void getSettingsTest() throws AdminException {
    RescheduleManager manager = getManager(new TestClusterControllerStub());

    RescheduleSettings s = manager.getSettings();

    assertEquals(20, s.getRescheduleLimit());
    assertEquals("30s,1m,5m,15m,30m,1h,6h,12h,1d:*", s.getDefaultReschedule());

    Collection<Reschedule> reschedules = s.getReschedules();
    assertNotNull(reschedules);
    assertEquals(2, reschedules.size());

    Iterator<Reschedule> iter = reschedules.iterator();

    assertEquals(new Reschedule("30s,11m,15m", 8), iter.next());
    assertEquals(new Reschedule("30s,1m,5m,15m,30m,1h,6h,12h,1d", 1028, 255, 20, 1027, 88, 100, 69), iter.next());
  }

  @Test
  public void updateSettingsTest() throws AdminException, XmlConfigException {
    // Загружаем первоначальный конфиг
    XmlConfig cfg = new XmlConfig();
    cfg.load(configFile);

    TestClusterControllerStub clusterController = new TestClusterControllerStub();
    RescheduleManager config1 = getManager(clusterController);
    RescheduleSettings s = config1.getSettings();
    s.setRescheduleLimit(20);

    config1.updateSettings(s);

    // Проверяем, что в директории backup появились файлы
    assertFalse(backupDir.delete()); // Не можем удалить директорию т.к. там появились файлы

    // Снова загружаем конфиг
    XmlConfig cfg1 = new XmlConfig();
    cfg1.load(configFile);

    // Проверяем эквивалентность первоначальному конфигу.
    assertEquals(cfg, cfg1);
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    RescheduleManager manager = getManager(new ClusterControllerImpl());

    Map<Integer, SmscConfigurationStatus> states = manager.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  public class ClusterControllerImpl extends TestClusterControllerStub {
    public ConfigState getMainConfigState() throws AdminException {
      long now = configFile.lastModified();
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 1);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }
}
