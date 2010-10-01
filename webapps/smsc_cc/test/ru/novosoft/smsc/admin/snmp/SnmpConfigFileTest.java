package ru.novosoft.smsc.admin.snmp;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.config.ManagedConfigHelper;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.Map;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

/**
 * @author Artem Snopkov
 */
public class SnmpConfigFileTest {

  private File configFile;

  @Before
  public void before() throws IOException {
    configFile = TestUtils.exportResourceToRandomFile(SnmpManagerImplTest.class.getResourceAsStream("snmp.xml"), ".snmp");
  }

  @After
  public void after() {
    if (configFile != null)
      configFile.delete();
  }

  private SnmpSettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new SnmpConfigFile());
  }

  private void saveSettings(SnmpSettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new SnmpConfigFile(), s);
  }

  @Test
  public void testSave() throws Exception {
    SnmpSettings s = loadSettings();
    saveSettings(s);

    testLoad();
  }

  @Test
  public void testLoad() throws Exception {
    SnmpSettings m = loadSettings();

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
}
