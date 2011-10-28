package ru.novosoft.smsc.admin.snmp;


import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerStub;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class SnmpManagerImplTest {

  private File configFile, backupDir;
  private MemoryFileSystem fs = new MemoryFileSystem();

  @Before
  public void before() throws IOException, AdminException {
    configFile = fs.createNewFile("snmp.xml", SnmpManagerImplTest.class.getResourceAsStream("snmp.xml"));
    backupDir = fs.mkdirs("backup");
  }

  private SnmpManagerImpl getManager(ClusterController cc) throws AdminException {
    return new SnmpManagerImpl(configFile, backupDir, new File(""), cc, fs);
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    SnmpManagerImpl m = getManager(new ClusterControllerImpl());

    Map<Integer, SmscConfigurationStatus> states = m.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  @Test
  public void nullGetStatusForSmscs() throws AdminException {
    SnmpManagerImpl m = getManager(new ClusterControllerImpl1());

    Map<Integer, SmscConfigurationStatus> states = m.getStatusForSmscs();

    assertTrue(states.isEmpty());
  }

  public class ClusterControllerImpl extends TestClusterControllerStub {
    public ConfigState getSnmpConfigState() throws AdminException {
      long now = fs.lastModified(configFile);
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 1);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }

  public class ClusterControllerImpl1 extends TestClusterControllerStub {
    public ConfigState getSnmpConfigState() throws AdminException {
      return null;
    }
  }
}
