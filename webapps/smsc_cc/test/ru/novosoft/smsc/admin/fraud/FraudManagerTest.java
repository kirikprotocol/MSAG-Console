package ru.novosoft.smsc.admin.fraud;

import org.junit.*;

import static org.junit.Assert.*;
import static org.junit.Assert.assertEquals;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerStub;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.util.Address;
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
  public void before() throws IOException, AdminException {
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

  private FraudManager getManager(ClusterController cc) throws AdminException {
    FraudManager fm = new FraudManager(configFile, backupDir, cc, FileSystem.getFSForSingleInst());
    return fm;
  }

  @Test
  public void loadTest() throws AdminException {
    FraudSettings fm = getManager(new TestClusterControllerStub()).getSettings();

    assertEquals(1, fm.getTail());
    assertTrue(fm.isEnableCheck());
    assertFalse(fm.isEnableReject());
    assertEquals(2, fm.getWhiteList().size());
    assertTrue(fm.getWhiteList().contains(new Address("12232321")));
    assertTrue(fm.getWhiteList().contains(new Address("9139495113")));
  }

  @Test
  public void updateTest() throws AdminException {
    TestClusterControllerStub clusterController = new TestClusterControllerStub();
    FraudManager fm1 = getManager(clusterController);
    FraudSettings settings1 = fm1.getSettings();

    settings1.setTail(5);
    settings1.setEnableCheck(false);
    settings1.setEnableReject(true);
    Collection<Address> whiteList = new ArrayList<Address>();
    Collections.addAll(whiteList, new Address("23123"), new Address("4324"), new Address("5564"));
    settings1.setWhiteList(whiteList);

    fm1.updateSettings(settings1);

    FraudManager fm = getManager(clusterController);
    FraudSettings settings = fm.getSettings();

    assertEquals(5, settings.getTail());
    assertFalse(settings.isEnableCheck());
    assertTrue(settings.isEnableReject());
    assertEquals(3, settings.getWhiteList().size());
    assertTrue(settings.getWhiteList().contains(new Address("23123")));
    assertTrue(settings.getWhiteList().contains(new Address("4324")));
    assertTrue(settings.getWhiteList().contains(new Address("5564")));

    assertFalse(backupDir.delete());
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    FraudManager fm = getManager(new ClusterControllerImpl());

    Map<Integer, SmscConfigurationStatus> states = fm.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  public class ClusterControllerImpl extends TestClusterControllerStub {
    public ConfigState getFraudConfigState() throws AdminException {
      long now = configFile.lastModified();
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 1);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }


}
