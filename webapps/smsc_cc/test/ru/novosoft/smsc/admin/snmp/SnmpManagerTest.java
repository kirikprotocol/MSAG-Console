package ru.novosoft.smsc.admin.snmp;


import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.*;
import static org.junit.Assert.assertEquals;

/**
 * @author Artem Snopkov
 */
public class SnmpManagerTest {

  private File configFile, backupDir;

  @Before
  public void before() throws IOException {
    configFile = TestUtils.exportResourceToRandomFile(SnmpManagerTest.class.getResourceAsStream("snmp.xml"), ".snmp");
    backupDir = TestUtils.createRandomDir(".snmpbackup");
  }

  @After
  public void after() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  private SnmpManager getManager(ClusterController cc) throws AdminException {
    SnmpManager m = new SnmpManager(configFile, backupDir, cc, FileSystem.getFSForSingleInst());
    m.reset();
    return m;
  }

  private void validate(SnmpManager m) throws AdminException {
    assertEquals(300, m.getCounterInterval());

    SnmpObject def = m.getDefaultSnmpObject();
    assertEquals(false, def.isEnabled());
    assertEquals(SnmpSeverity.NORMAL, def.getSeverityRegister());
    assertEquals(SnmpSeverity.WARNING, def.getSeverityUnregister());
    assertNull(def.getSeverityUnregisterFailed());
    assertNull(def.getSeverityRegisterFailed());
    assertEquals(new SnmpCounter(10, 20, 30, 40), def.getCounterOther());
    assertEquals(new SnmpCounter(10, 20, 30, 40), def.getCounter0014());
    assertEquals(new SnmpCounter(10, 20, 30, 40), def.getCounter000d());
    assertEquals(new SnmpCounter(10, 20, 30, 40), def.getCounterFailed());
    assertEquals(new SnmpCounter(10, 20, 30, 40), def.getCounterRejected());
    assertEquals(new SnmpCounter(10, 20, 30, 40), def.getCounterSDP());
    assertEquals(new SnmpCounter(10, 20, 30, 40), def.getCounterRetried());
    assertEquals(new SnmpCounter(10, 20, 30, 40), def.getCounter0058());
    assertEquals(new SnmpCounter(10, 20, 30, 40), def.getCounterTempError());
    assertEquals(new SnmpCounter(50, 40, 30, 20), def.getCounterAccepted());
    assertEquals(new SnmpCounter(50, 40, 30, 20), def.getCounterDelivered());

    Map<String, SnmpObject> objects = m.getSnmpObjects();
    assertNotNull(objects);
    assertEquals(2, objects.size());

    SnmpObject my = objects.get("MY");
    assertNotNull(my);
    assertEquals(true, my.isEnabled());

    assertEquals(SnmpSeverity.NORMAL, my.getSeverityRegister());
    assertEquals(SnmpSeverity.OFF, my.getSeverityUnregister());
    assertEquals(SnmpSeverity.OFF, my.getSeverityUnregisterFailed());
    assertEquals(SnmpSeverity.OFF, my.getSeverityRegisterFailed());

    assertEquals(new SnmpCounter(0, 0, 0, 0), my.getCounterOther());
    assertEquals(new SnmpCounter(0, 0, 0, 0), my.getCounter0014());
    assertEquals(new SnmpCounter(0, 0, 0, 0), my.getCounter000d());
    assertEquals(new SnmpCounter(1, 2, 3, 4), my.getCounterFailed());
    assertEquals(new SnmpCounter(0, 0, 0, 0), my.getCounterRejected());
    assertEquals(new SnmpCounter(0, 0, 0, 0), my.getCounterSDP());
    assertEquals(new SnmpCounter(0, 0, 0, 0), my.getCounterRetried());
    assertEquals(new SnmpCounter(0, 0, 0, 0), my.getCounter0058());
    assertEquals(new SnmpCounter(0, 0, 0, 0), my.getCounterTempError());
    assertEquals(new SnmpCounter(12, 32, 43, 4), my.getCounterAccepted());
    assertEquals(new SnmpCounter(0, 0, 0, 0), my.getCounterDelivered());
  }

  @Test
  public void loadTest() throws AdminException {
    SnmpManager m = getManager(new TestClusterController());

    validate(m);
  }

  @Test
  public void applyTest() throws AdminException {
    TestClusterController controller = new TestClusterController();
    SnmpManager m = getManager(controller);
    assertFalse(m.isChanged());

    SnmpObject def = m.getDefaultSnmpObject();
    def.setCounterRejected(10, 20, 30, 40);
    m.setDefaultSnmpObject(def);

    assertTrue(m.isChanged());

    m.apply();

    assertTrue(controller.applySnmpCalled);
    assertFalse(m.isChanged());
    validate(m);
  }

  @Test
  public void resetTest() throws AdminException {
    SnmpManager m = getManager(new TestClusterController());

    SnmpObject def = m.getDefaultSnmpObject();
    def.setCounterRejected(40, 40, 40, 40);
    m.setDefaultSnmpObject(def);

    m.reset();

    assertFalse(m.isChanged());

    validate(m);
  }

  @Test
  public void resetFailedTest() throws AdminException {
    SnmpManager m = getManager(new TestClusterController());

    SnmpObject def = m.getDefaultSnmpObject();
    def.setCounterRejected(40, 40, 40, 40);
    m.setDefaultSnmpObject(def);

    configFile.delete();

    try {
      m.reset();
      assertFalse(true);
    } catch (AdminException e) {
    }

    assertTrue(m.isChanged());
    assertEquals(new SnmpCounter(40,40,40,40), m.getDefaultSnmpObject().getCounterRejected());
  }

  @Test
  public void getLastChangedTest() throws AdminException, InterruptedException {
    SnmpManager m = getManager(new TestClusterController());
    assertEquals(-1, m.getLastChangeTime());

    long now = System.currentTimeMillis();

    m.setCounterInterval(200);
    assertTrue(m.getLastChangeTime() >= now);

    Thread.sleep(10);

    now = System.currentTimeMillis();

    assertFalse(m.getLastChangeTime() >= now);
    m.reset();
    assertTrue(m.getLastChangeTime() >= now);
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    SnmpManager m = getManager(new ClusterControllerImpl());

    Map<Integer, SmscConfigurationStatus> states = m.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  public class ClusterControllerImpl extends TestClusterController {
    public ConfigState getSnmpConfigState() throws AdminException {
      long now = configFile.lastModified();
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 1);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }
}
