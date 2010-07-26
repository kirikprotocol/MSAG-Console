package ru.novosoft.smsc.admin.fraud;

import org.junit.*;

import static org.junit.Assert.*;
import static org.junit.Assert.assertEquals;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import testutils.TestUtils;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;

import java.io.File;
import java.io.IOException;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class FraudManagerTest {

  private File configFile, backupDir;

  @Before
  public void before() throws IOException {
    configFile = TestUtils.exportResourceToRandomFile(FraudManagerTest.class.getResourceAsStream("fraud.xml"), ".fraud");
    backupDir = TestUtils.createRandomDir(".fraudbackup");
  }

  @After
  public void after() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  @Test
  public void loadTest() throws AdminException {
    FraudManager fm = new FraudManager(configFile, backupDir, new TestClusterController(), FileSystem.getFSForSingleInst());
    assertFalse(fm.isChanged());

    assertEquals(1, fm.getTail());
    assertTrue(fm.isEnableCheck());
    assertFalse(fm.isEnableReject());
    assertEquals(2, fm.getWhiteList().size());
    assertTrue(fm.getWhiteList().contains("12232321"));
    assertTrue(fm.getWhiteList().contains("9139495113"));
  }

  @Test
  public void saveTest() throws AdminException {
    TestClusterController clusterController = new TestClusterController();
    FraudManager fm1 = new FraudManager(configFile, backupDir, clusterController, FileSystem.getFSForSingleInst());

    fm1.setTail(5);
    fm1.setEnableCheck(false);
    fm1.setEnableReject(true);
    Collection<String> whiteList = new ArrayList<String>();
    Collections.addAll(whiteList, "23123", "4324", "5564");
    fm1.setWhiteList(whiteList);

    assertTrue(fm1.isChanged());

    fm1.apply();

    assertFalse(fm1.isChanged());

    assertTrue(clusterController.applyFraudCalled);

    FraudManager fm = new FraudManager(configFile, backupDir, clusterController, FileSystem.getFSForSingleInst());

    assertEquals(5, fm.getTail());
    assertFalse(fm.isEnableCheck());
    assertTrue(fm.isEnableReject());
    assertEquals(3, fm.getWhiteList().size());
    assertTrue(fm.getWhiteList().contains("23123"));
    assertTrue(fm.getWhiteList().contains("4324"));
    assertTrue(fm.getWhiteList().contains("5564"));

    assertFalse(backupDir.delete());
  }

  @Test
  public void resetTest() throws AdminException {
    FraudManager fm = new FraudManager(configFile, backupDir, new TestClusterController(), FileSystem.getFSForSingleInst());
    assertFalse(fm.isChanged());

    fm.setTail(5);
    fm.setEnableCheck(false);
    fm.setEnableReject(true);
    fm.setWhiteList(new ArrayList<String>());

    assertTrue(fm.isChanged());

    fm.reset();

    assertFalse(fm.isChanged());

    assertEquals(1, fm.getTail());
    assertTrue(fm.isEnableCheck());
    assertFalse(fm.isEnableReject());
    assertEquals(2, fm.getWhiteList().size());
    assertTrue(fm.getWhiteList().contains("12232321"));
    assertTrue(fm.getWhiteList().contains("9139495113"));
  }

  @Test
  public void resetFailedTest() throws AdminException {
    FraudManager fm = new FraudManager(configFile, backupDir, new TestClusterController(), FileSystem.getFSForSingleInst());
    assertFalse(fm.isChanged());

    fm.setTail(5);
    fm.setEnableCheck(false);
    fm.setEnableReject(true);
    fm.setWhiteList(new ArrayList<String>());

    assertTrue(fm.isChanged());

    configFile.delete();

    try {
      fm.reset();
      assertFalse(true);
    } catch (AdminException e) {
    }

    assertTrue(fm.isChanged());

    assertEquals(5, fm.getTail());
    assertFalse(fm.isEnableCheck());
    assertTrue(fm.isEnableReject());
    assertEquals(0, fm.getWhiteList().size());
  }

  @Test
  public void setWhiteListTest() throws AdminException {
    FraudManager fm = new FraudManager(configFile, backupDir, new TestClusterController(), FileSystem.getFSForSingleInst());
    Collection<String> whiteList = new ArrayList<String>();

    // Корректный адрес
    try {
      whiteList.clear();
      whiteList.add("791239239");
      fm.setWhiteList(whiteList);
    } catch (IllegalArgumentException e) {
      assertFalse(true);
    }

    // Слишком короткий адрес
    try {
      whiteList.clear();
      whiteList.add("");
      fm.setWhiteList(whiteList);
      assertFalse(true);
    } catch (IllegalArgumentException e) {
    }

    // Слишком длинный адрес
    try {
      whiteList.clear();
      whiteList.add("1234567890123456");
      fm.setWhiteList(whiteList);
      assertFalse(true);
    } catch (IllegalArgumentException e) {
    }

    // Адрес, содержащий буквы
    try {
      whiteList.clear();
      whiteList.add("fdfs");
      fm.setWhiteList(whiteList);
      assertFalse(true);
    } catch (IllegalArgumentException e) {
    }
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    FraudManager fm = new FraudManager(configFile, backupDir, new ClusterControllerImpl(), FileSystem.getFSForSingleInst());

    Map<Integer, SmscConfigurationStatus> states = fm.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  public class ClusterControllerImpl extends TestClusterController {
    public ConfigState getFraudConfigState() throws AdminException {
      long now = configFile.lastModified();
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 1);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }


}
